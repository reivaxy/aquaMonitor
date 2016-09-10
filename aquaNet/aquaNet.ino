
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// The aquaMonitorSecret.h should be stored in a same-name sub directory of
// your libraries directory. It's obviously not in the git repository :)
// It should define these:
// #define DEFAULT_WIFI_SSID "your-home-ssid"
// #define DEFAULT_WIFI_PWD "your-home-network-password"
// #define DEFAULT_STAT_HOST "http://www.your-stat-host.com"
// #define DEFAULT_STAT_PATH "/aquaStat.php"
#include "aquaMonitorSecret.h"

// File with all messages
#include "aquaNetMessages.h"

// Structure to hold all wifi-related configuration
#define CONFIG_VERSION 'A'
#define CONFIG_ADDRESS 16
#define DEFAULT_AP_SSID "aquaMonitor"    // AP ssid by default, known by all devices, can be changed
#define DEFAULT_AP_PWD  "aquaPwd"        // AP pwd by default so that other devices can connect
struct eepromConfig {
  unsigned char version; // Always keep it as first member when modifying this structure
  char statisticsHost[50]; // hostname to send statistics to; e.g. http://www.myStats.com
  char statisticsPath[50]; // path on the host to send statistics to; e.g. cgi-bin/recordStats
  unsigned long statisticsInterval = 0; // number of milliseconds between two statistic sending
  char homeSsid[20];       // ssid to connect to
  char homePwd[65];        // Password for wifi connection
  char APSsid[20];         // ssid to create as AP for the other modules
  char APPwd[65];          // Password for the AP.
  char deviceName[20];     // Name of this device, to identify it within a network
} config;

// Init config structure from EEPROM
// If not in EEPROM or version has changed from what is stored in EEPROM, 
// reset config to default values, and save it to EEPROM for next time
void readConfig() {
  unsigned char i;

  unsigned int configSize = sizeof(config);
  Serial.print("Taille config: ");
  Serial.println(configSize);
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
    strcpy(config.homeSsid, DEFAULT_WIFI_SSID);
    strcpy(config.homePwd, DEFAULT_WIFI_PWD);
    strcpy(config.APSsid, DEFAULT_AP_SSID);
    strcpy(config.APPwd, DEFAULT_AP_PWD);

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

ESP8266WebServer server(80);
WiFiClient client;
MDNSResponder mdns;
unsigned long startTime = millis();
int clientConnected = 0; 
boolean homeWifiConnected = false;
char message[60];

void setup(void){
  int timeout = 40;
  char ip[20];

  unsigned int configSize = sizeof(config);
  EEPROM.begin(configSize);  // Config is read from and stored to EEPROM
  Serial.begin(115200);
  delay(3000); // delay to connect monitor
  Serial.println("");
  readConfig();

  // Set the accesspoint
  // `WiFi.mode(m)`: set mode to `WIFI_AP`, `WIFI_STA`, `WIFI_AP_STA` or `WIFI_OFF`.
  // call `WiFi.softAP(ssid)` to set up an open network
  // call `WiFi.softAP(ssid, password)` to set up a WPA2-PSK network (password should be at least 8 characters)
  // `WiFi.macAddress(mac)` is for STA, `WiFi.softAPmacAddress(mac)` is for AP.
  // `WiFi.localIP()` is for STA, `WiFi.softAPIP()` is for AP.
  // `WiFi.printDiag(Serial)` will print out some diagnostic info
  Serial.println("Creating AP");
  Serial.println(config.APSsid);
  Serial.println(config.APPwd);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(config.APSsid, config.APPwd);

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
      Serial.println(">MDNS responder started");
    }
  }
  server.on("/", [](){
    char name[50];
    char value[50];
    server.argName(0).toCharArray(name, 50);
    server.arg(0).toCharArray(value, 50);
    sprintf(message, "%s=%s", name, value);
    server.send(200, "text/html", message);
    delay(1000);
  });

  server.begin();
  Serial.println(">HTTP server started");
  WiFi.printDiag(Serial);
}


void loop(void){
  server.handleClient();
  char request[100];
  int now = millis();
  if ((now - startTime > 20000) && (config.statisticsHost[0] !=0) && homeWifiConnected){
    startTime = now;
    Serial.println("Connecting to Host");
    Serial.println(config.statisticsHost);
    if (client.connect(config.statisticsHost, 80)) {
      clientConnected = 1;
      Serial.println("Connected to Host");
      // Make a HTTP request:
      sprintf(request, "GET %s?time=%d&%s HTTP/1.1", config.statisticsPath, startTime, message);
      Serial.println(request);
      client.println(request);
      sprintf(request, "Host: %s", config.statisticsHost);
      client.println(request);
      client.println("Connection: close");
      client.println();
    }
  }
  // if the server's disconnected, stop the client:
  if (clientConnected == 1 && !client.connected()) {
    Serial.println("Disconnecting from host.");
    client.stop();
    clientConnected = 0;
  }
}
