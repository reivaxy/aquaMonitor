
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
// The aquaMonitorSecret.h should be stored in a same-name sub directory of
// your libraries directory. It's obviously not in the git repository :)
// It should define these:
// #define DEFAULT_WIFI_SSID "your-home-ssid"
// #define DEFAULT_WIFI_PWD "your-home-network-password"
// #define DEFAULT_STAT_HOST "http://www.your-stat-host.com"
// #define DEFAULT_STAT_PATH "/aquaStat.php"
#include "aquaMonitorSecret.h"

// technical messages between esp and arduino
#include "interComMsg.h"
#include "common.h"
#include "html.h"

// localizable messages intended to be displayed/sent to usb serial
#include "aquaNetMessages.h"

// Structure to hold all wifi-related configuration
#define CONFIG_VERSION 'F'
#define DEFAULT_AP_SSID "aquaMonitor"    // AP ssid by default, known by all devices, can be changed
#define DEFAULT_AP_PWD  "aquaPassword"   // 8 CHAR OR MORE !! AP pwd by default so that other devices can connect
#define DEFAULT_STAT_INTERVAL  300000    // update stats every 5 minutes by default
#define DEFAULT_ARDUINO_CHECK_INTERVAL  2000    // check incoming message from arduino every x milliseconds by default
struct eepromConfig {
  unsigned char version; // Always keep it as first member when modifying this structure
  char statisticsHost[50]; // hostname to send statistics to; e.g. http://www.myStats.com
  char statisticsPath[50]; // path on the host to send statistics to; e.g. cgi-bin/recordStats
  char logPath[50]; // path on the host to send logs (incoming sms, voice call numbers... ?)
  unsigned long statisticsInterval = 0; // number of milliseconds between two statistic sending
  unsigned long arduinoCheckInterval = 0; // number of milliseconds between two incoming arduino message checks
  char homeSsid[20];       // ssid to connect to (home network)
  char homePwd[65];        // Password for wifi connection
  char APSsid[20];         // ssid to create as AP for the other modules
  char APPwd[65];          // Password for the AP.
  char deviceName[20];     // Name of this device, to identify it within a network
} config;

ESP8266WebServer server(80);
WiFiClient client;
MDNSResponder mdns;
int clientConnected = 0;
boolean homeWifiConnected = false;

char serialMessage[MAX_SERIAL_INPUT_MESSAGE];
unsigned long lastStatSent = 0;
unsigned long lastArduinoCheck = 0;
char aquaStatus[500]; // Json string containing data from arduino

// return true if current time is after given time + delay
boolean checkElapsedDelay(unsigned long now, unsigned long lastTime, unsigned long delay) {
  unsigned long elapsed = now - lastTime;
  boolean result = false;
  // millis() overflows unsigned long after about 50 days => 0  but since unsigned,
  // no problem !
  if(elapsed >= delay){
    result = true;
  }
  if(lastTime == 0) {
    result = true;
  }
  return result;
}

// Init config structure from EEPROM
// If not in EEPROM or version has changed from what is stored in EEPROM,
// reset config to default values, and save it to EEPROM for next time
void readConfig() {
  unsigned char i;

  unsigned int configSize = sizeof(config);
  unsigned int cptr = 0;
  byte *configPtr = (byte *)&config;
  for(cptr = 0; cptr < configSize; cptr++) {
    *(configPtr ++) = EEPROM.read(cptr);
  }

  if(config.version != CONFIG_VERSION) {
    Serial.println(WIFI_CONFIG_INIT);
    config.version = CONFIG_VERSION;
    strcpy(config.statisticsHost, DEFAULT_STAT_HOST);
    strcpy(config.statisticsPath, DEFAULT_STAT_PATH);
    strcpy(config.logPath, DEFAULT_LOG_PATH);
    strcpy(config.homeSsid, DEFAULT_WIFI_SSID);
    strcpy(config.homePwd, DEFAULT_WIFI_PWD);
    strcpy(config.APSsid, DEFAULT_AP_SSID);
    strcpy(config.APPwd, DEFAULT_AP_PWD);
    strcpy(config.deviceName, "Aquarium");
    config.statisticsInterval = DEFAULT_STAT_INTERVAL; // number of milliseconds between two statistic sending
    config.arduinoCheckInterval = DEFAULT_ARDUINO_CHECK_INTERVAL; // number of milliseconds between two arduino msg checks

    // Save the configuration to EEPROM
    Serial.println(WIFI_CONFIG_SAVING);
    configPtr = (byte *)&config;
    for(cptr = 0; cptr < configSize; cptr++) {
      EEPROM.write(cptr, *(configPtr++));
    }
    EEPROM.commit();

    Serial.println(WIFI_CONFIG_SAVED);
  }
}

void setup(void){
  int timeout = 40;
  char ip[20];
  char message[100];

  unsigned int configSize = sizeof(config);
  serialMessage[0] = 0;

  strcpy(aquaStatus, "[]");  // No data yet
  EEPROM.begin(configSize);  // Config is read from and stored to EEPROM
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // ap could be active from previous time

  delay(10000); // delay to connect monitor
  Serial.println("Init aquaNet");
  readConfig();

  // Connect to home network if any
  if(config.homeSsid[0] != 0) {
    WiFi.begin(config.homeSsid, config.homePwd);
    Serial.println(CONNECTING_HOME_NETWORK);

    // Wait for connection, with timeout
    while ((WiFi.status() != WL_CONNECTED) && timeout --) {
      delay(500);
    }
    if(timeout != 0) {
      homeWifiConnected = true;
      sprintf(message, CONNECTED_HOME_NETWORK, config.homeSsid);
      Serial.println(message);
    } else {
      Serial.println(CONNECTION_TIMED_OUT);
    }

    Serial.print(ACCESS_IP);
    Serial.println(WiFi.localIP());

    if (mdns.begin("esp8266", WiFi.localIP())) {
      Serial.println("MDNS responder started");
    }
  }

  server.on("/", [](){
    printHTMLPage();
  });

  server.on("/msgArduino", [](){
    // Should be only one param : the command to send to arduino
    char command[200];
    server.arg(0).toCharArray(command, 200);
    sendArduinoCommand(command);
    server.send(200, "text/plain", "");
  });

  server.on("/msgESP", [](){
    // We assume there is only one param
    char command[200];
    server.arg(0).toCharArray(command, 200);
    processMessage(command);
    server.send(200, "text/plain", "");
  });

  server.on("/getData.json", [](){
    server.send(200, "text/plain", aquaStatus);
  });

  // Ask Arduino if it is equipped with GSM
  sendArduino(REQUEST_IF_GSM);

  server.begin();
  Serial.println("HTTP server started");
  //WiFi.printDiag(Serial);
}

void printHTMLPage() {
  server.send(200, "text/html", html);
  delay(1000);
}
// Create Access Point (once arduino confirmed it's equipped with GSM module
void startAP() {
  // Set the accesspoint only if module equipped with GSM
  // `WiFi.mode(m)`: set mode to `WIFI_AP`, `WIFI_STA`, `WIFI_AP_STA` or `WIFI_OFF`.
  // call `WiFi.softAP(ssid)` to set up an open network
  // call `WiFi.softAP(ssid, password)` to set up a WPA2-PSK network (password should be at least 8 characters)
  // `WiFi.macAddress(mac)` is for STA, `WiFi.softAPmacAddress(mac)` is for AP.
  // `WiFi.localIP()` is for STA, `WiFi.softAPIP()` is for AP.
  // `WiFi.printDiag(Serial)` will print out some diagnostic info
  Serial.println(CREATING_AP);
  Serial.println(config.APSsid);
  Serial.println(config.APPwd);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(config.APSsid, config.APPwd);
}



void loop(void) {
  unsigned long now = millis();
  boolean result ;
  server.handleClient();
  if ((config.statisticsHost[0] !=0) && homeWifiConnected && checkElapsedDelay(now, lastStatSent, config.statisticsInterval/10)) {
    sendArduino(REQUEST_MEASURES);
    lastStatSent = now;  // Don't wait for response to not request again next loop
  }

  if(readFromSerial(&Serial, serialMessage, 500)) {
    processMessage(serialMessage);
    serialMessage[0] = 0;
  }

}

// Process a message from the arduino. It should contain two parts separated with ':'
void processMessage(char *message) {
  char *colonPosition;
  char *content;

  colonPosition = strchr(message, ':');
  if(colonPosition != NULL) {
    content = colonPosition + 1;
    // Check prefix for what needs to be done
    if(strncmp(message, REQUEST_IF_GSM, strlen(REQUEST_IF_GSM)) == 0) {
      // If module equipped with GSM, start Wifi Access Point
      if(content[0] == '1') {
        startAP();
      } else {
        Serial.println(CLOSING_AP);
        WiFi.mode(WIFI_STA); // Stop the access point
      }
    } else if(strncmp(message, REQUEST_MEASURES, strlen(REQUEST_MEASURES)) == 0) {
      // StaticJsonBuffer size : https://rawgit.com/bblanchon/ArduinoJson/master/scripts/buffer-size-calculator.html
      StaticJsonBuffer<600> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(content);
      root["name"] = config.deviceName;
      root["type"] = 0; // TODO
      root["localIP"] = WiFi.localIP().toString();
      root["APName"] = config.APSsid;
      root["APIP"] = WiFi.softAPIP().toString();
      // V3 will handle several modules => array of data. Already handled by webApp.
      // Will improve this when implementing V3. Good enough for now
      strcpy(aquaStatus, "[");
      char *firstChar = aquaStatus;
      firstChar++;
      root.printTo(firstChar, sizeof(aquaStatus) -1);
      strcat(aquaStatus, "]");
      Serial.println("");
      //writeToSerial(&Serial, aquaStatus, 500);
      sendStat();
    }
  }
}

void sendStat() {
return;
  char request[2100]; // Todo use String
  char param[2000];
  if (client.connect(config.statisticsHost, 80)) {
    clientConnected = 1;
    sprintf(request, "GET %s?stat=%s HTTP/1.1", config.statisticsPath, urlEncode(aquaStatus, param, sizeof(param)));
    Serial.println("Logged Stats");
    client.println(request);
    sprintf(request, "Host: %s", config.statisticsHost);
    client.println(request);
    client.println("Connection: close");
    client.println();
  }
  // TODO: what is the purpose of this ?
  // if the server's disconnected, stop the client:
  if (clientConnected == 1 && !client.connected()) {
    Serial.println("Disconnecting from host.");
    client.stop();
    clientConnected = 0;
  }
}

// Send a request to Arduino.
// Requests for esp-arduino specific conversation start with '#'
// It's possible to send arduino commands with '@' as first character
// Other messages sent to arduino via Serial will be forwarded to arduino's USB serial
void sendArduino(char *msg) {
  char message[200];
  if(*msg == '@') {
    // Arduino command. i.e. status, or config
    sprintf(message, "%s", msg);
  } else {
    // ESP command sent to arduino, i.e. chkGSM ...
    sprintf(message, "#%s", msg);
  }
  Serial.println(message);
}

// Send an arduino command to Arduino.
// Requests that are commands for arduino start with '@'
// Other messages sent to arduino via Serial will be forwarded to arduino's USB serial
void sendArduinoCommand(char *msg) {
  char message[200];
  sprintf(message, "@%s", msg);
  Serial.println(message);
}

char *urlEncode(char *toEncode, char *encoded, int size) {
  // char to not encode
  char notEncode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
  char *scanIn = toEncode;
  char *scanOut = encoded;
  while(*scanIn) {
    if (strchr(notEncode, *scanIn)) {
      *scanOut = *scanIn;
      *(++scanOut) = 0;
    } else {
      char buf[4];
      sprintf(buf, "%%%02X", *scanIn);
    //  if((strlen(encoded) + strlen(buf)) < size +1) {
        strcat(scanOut, buf);
        scanOut += 3;
//      } else {
//        Serial.println("Message to encode too big");
//      }
    }
    scanIn ++;
  }
  return encoded;
}