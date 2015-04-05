// BOF preprocessor bug prevent - insert me on top of your arduino-code
#if 1
__asm volatile ("nop");
#endif

#include <OneWire.h>
#include <GSM.h>
#include <EEPROMex.h>
#include <EEPROMvar.h>
#include <Wire.h>
#include "DS1307.h"


// 'admin' phone number defined outside of open source file haha !
#include <remoteNumber.h>

// Compilation directives to enable/disable stuff like IR support
// Beware, 'includes' for the matching libraries need to be commented / uncommented
// In the first #if using these flags
#define WITH_IR_SUPPORT false      // About 7k prog, 200B RAM
#define WITH_LCD_SUPPORT true      // About 1,6k prog, 30B RAM


// Strings to store in progmem space to free variable space
#include "./progmemStrings.h"

#define PHONE_NUMBER_LENGTH 15
#define MAX_PHONE_NUMBERS 4
#define CONFIG_ADDRESS 0

// Change this version to reset the EEPROM saved configuration
// when the structure changes
#define CONFIG_VERSION 4

struct phoneConfig {
  unsigned char permissionFlags;
  char number[PHONE_NUMBER_LENGTH + 1];
  unsigned long lastAlertSmsTime;  // When was last time (millis()) an alert SMS sent to that number
  unsigned long minAlertInterval;  // minimum interval in seconds between 2 alert SMS to avoir flooding 
};

struct eepromConfig {
  unsigned int version;          // Version for this config structure and default values. Always keep as first structure member 
  int temperatureAdjustment;     // Signed offset to add to temperature measure to adjust it
  int temperatureHighThreshold;  // Alert will be sent if temperature above this value (centiCelsius: 2550 is 25,50°)
  int temperatureLowThreshold;   // Alert will be sent if temperature below this value (centiCelsius: 2400 is 24°)
  int lightThreshold;            // Alert will be sent if light below this value within on/off hours (see below)
  byte lightOnHour;              // Hour after which light should be above the threshold
  byte lightOnMinute;            // Minute ...
  byte lightOffHour;             // Hour after which light below threshold won't be considered an alert
  byte lightOffMinute;           // Minute ...
  phoneConfig registeredNumbers[MAX_PHONE_NUMBERS];  // Phone numbers to send alerts to, according to their subscriptions and permissions
} config;

// Flags for services subscriptions and permissions
#define FLAG_SERVICE_ALERT   0x01
#define FLAG_SERVICE_EVENT   0x02
#define FLAG_SERVICE_SUB     0x04
#define FLAG_SERVICE_UNSUB   0x08
#define FLAG_ADMIN           0x80

#if WITH_IR_SUPPORT
// #include are processed no matter what (known bug) : comment or uncomment them is the only way
//#include <IRremote.h>
// Init IR
#define IR_PIN 5
IRrecv reception_ir(IR_PIN);

// These are the codes sent by Remote used to trigger stuff. Replace with your own
// TODO: Some future feature may need a sequence of keys. Not handled for now
#define IR_DISPLAY_PHONE_NUMBER 0xF720DF       // (R key on xanlite remote, for now)
#define IR_INCR_LIGHT_THRESHOLD 0xF700FF       // (light+ on xanlite remote, for now)
#define IR_DECR_LIGHT_THRESHOLD 0xF7807F       // (light- on xanlite remote, for now)
#define IR_INCR_TEMP_ADJUSTMENT 0xF740BF       // (OFF on xanlite remote, for now)
#define IR_DECR_TEMP_ADJUSTMENT 0xF7C03F       // (ON on xanlite remote, for now)
#define IR_DISPLAY_THRESHOLDS   0xF7A05F       // (V on xanlite remote, for now)
#define IR_DISPLAY_TEMP_ADJUST  0xF7609F       // (B on xanlite remote, for now)
#define IR_SAVE_TO_EEPROM       0xF7E817    // ('Lent' on xanlite remote, for now)
#define IR_LOG_CONFIG           0xF728D7    // jaune clair
// When a key is maintained pressed, a different code is sent, and we want to repeat the operation
#define IR_REPEAT_CODE 0xFFFFFFFF
unsigned long previousCode = 0;

#define IR_DISPLAY_TIMEOUT 250
unsigned long lastIrDisplay = 0;
#endif

// GSM
// initialize the library instance
#define PINNUMBER "" // Pin number for the SIM CARD
GSM gsmAccess; // include a 'true' parameter for debug enabled
GSM_SMS sms;

// LCD
#if WITH_LCD_SUPPORT
// #include are processed no matter what (known bug) : comment or uncomment them is the only way
#include <LiquidCrystal.h>
// initialize the library with the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11 , 12);
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#endif

#define MSG_MAX_LENGTH 14

#define LIGHT_PIN 0
#define TEMPERATURE_PIN 6

/* DS18B20 Temperature chip i/o */
OneWire  ds(TEMPERATURE_PIN);
#define MAX_DS1820_SENSORS 1
byte addr[MAX_DS1820_SENSORS][8];

// Okay these globals are pretty bad and still the cause of a few bugs, but the
// small amount of variable space left me with little choice.
#define PROGMEM_MSG_MAX_SIZE 60
char progMemMsg[PROGMEM_MSG_MAX_SIZE + 1];
char temperatureMsg[20];
char lightMsg[20];

#define TEMPERATURE_CHECK_PERIOD 5000
unsigned long lastTemperatureCheck = millis() - TEMPERATURE_CHECK_PERIOD;

#define LIGHT_CHECK_PERIOD 5000
unsigned long lastLightCheck = millis() - LIGHT_CHECK_PERIOD;

// Check for incoming sms every 30 seconds
#define SMS_CHECK_PERIOD 30000
unsigned long lastSmsCheck = 0;

#define MIN_SMS_SEND_DELAY 30000
unsigned long lastSmsSent = 0;
// Max size of a received SMS
#define MAX_SMS_LENGTH 20
boolean gsmEnabled = !false;

boolean statusOK = true;

DS1307 clock; // The RTC handle to get date

// System initialization
void setup(void) {
  Serial.begin(9600);

  // Just in case some loop goes crazy, limit the number of EEPROM writes to save it
  // TODO : this should be removed some day
  EEPROM.setMaxAllowedWrites(100);

  // Initialise RTC
  clock.begin();
 
#if WITH_LCD_SUPPORT
  lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
  lcd.clear();
#endif

  print(0, 0, getProgMemMsg(INIT_AQUAMON_MSG));
  readConfig();

#if WITH_IR_SUPPORT
  print(0, 1, getProgMemMsg(INIT_IR_MSG));
  reception_ir.enableIRIn(); // init receiver
  delay(250);
#endif

  print(0, 1, getProgMemMsg(TEMP_INIT_MSG));
  if (!ds.search(addr[0]))
  {
    print(0, 1, getProgMemMsg(ADDR_ERR_MSG));
    ds.reset_search();
    delay(250);
  }
  // Delay for GSM init TODO : try removing this (power issues solved)
  if(gsmEnabled) {
    delay(7000);
  }
  // GSM connection state
  boolean notConnected = true && gsmEnabled;

  // Start GSM shield
  print(0, 0, getProgMemMsg(INIT_GSM_MSG));
  while(notConnected)
  {
    print(0, 1, getProgMemMsg(CONNECTING_GSM_MSG));
    if(gsmAccess.begin(PINNUMBER)==GSM_READY) {
      print(0, 1, getProgMemMsg(CONNECTED_GSM_MSG));
      notConnected = false;
    } else {
      print(0, 1, getProgMemMsg(NOT_CONNECTED_GSM_MSG));
      delay(200);
    }
  }
  delay(500);
#if WITH_LCD_SUPPORT
  lcd.clear();
#endif
}

// Returns pointer to the global string buffer holding a string read from PROGMEM
char * getProgMemMsg(int messageId) {
  strncpy_P(progMemMsg, (char*)pgm_read_word(&messages[messageId]), PROGMEM_MSG_MAX_SIZE);
  // Just in case
  progMemMsg[PROGMEM_MSG_MAX_SIZE] = 0;
  return progMemMsg;
}

void loop(void) {
  // RTC does not have an epoch field :(
  unsigned long now = millis();

#if WITH_IR_SUPPORT
  decode_results code;

  // Check if an IR code was received
  if (reception_ir.decode(&code)) {
    processIRCode(code.value);
    reception_ir.resume(); // be ready for next code
  }
  if((now - lastIrDisplay) >= IR_DISPLAY_TIMEOUT) {
    resetIRDisplay();
  }
#endif

  if(checkElapsedDelay(now, lastLightCheck, LIGHT_CHECK_PERIOD)) {
    statusOK = statusOK && checkLight();
    lastLightCheck = now;
  }
  if(checkElapsedDelay(now, lastTemperatureCheck, TEMPERATURE_CHECK_PERIOD)) {
    statusOK = statusOK && checkTemperature();
    lastTemperatureCheck = millis();
  }
  if(checkElapsedDelay(now, lastSmsCheck, SMS_CHECK_PERIOD)) {
    checkSMS();
    lastSmsCheck = now;
  }

  if(!statusOK && checkElapsedDelay(now, lastSmsSent, MIN_SMS_SEND_DELAY)) {
    sendAlert();
    statusOK = true;
    lastSmsSent = now;
  }
}

// return true if current time is after given time + delay
boolean checkElapsedDelay(unsigned long now, unsigned long lastTime, unsigned long delay) {
  unsigned long elapsed = now - lastTime;
  boolean result = false;

  // millis() overflows unsigned long after about 50 days => 0
  if(elapsed < 0) {
    elapsed += 0xFFFFFFFFul;
  }

  if(elapsed >= delay) {
    return true;
  }
  return result; 
}

// return true if temperature is within low and high thresholds
boolean checkTemperature() {
  boolean temperatureOK = true;
  int highByte, lowByte, tReading, signBit, tc_100, whole, fract;
  byte sensor = 0;

  byte i;
  byte present = 0;
  byte data[12];

  if ( OneWire::crc8( addr[sensor], 7) != addr[sensor][7]) {
    print(0, 0, getProgMemMsg(CRC_NOT_VALID_MSG));
  } else if ( addr[sensor][0] != 0x28) {
    print(0, 0, getProgMemMsg(FAMILY_MSG));
  } else {
    ds.reset();
    ds.select(addr[sensor]);
    ds.write(0x44,1);         // start conversion, with parasite power on at the end

    //delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr[sensor]);
    ds.write(0xBE);         // Read Scratchpad

    for ( i = 0; i < 9; i++) {
      // we need 9 bytes
      data[i] = ds.read();
    }

    lowByte = data[0];
    highByte = data[1];
    tReading = (highByte << 8) + lowByte;
    signBit = tReading & 0x8000;  // test most sig bit
    if (signBit) // negative
    {
      tReading = (tReading ^ 0xffff) + 1; // 2's comp
    }
    tc_100 = (6 * tReading) + tReading / 4;    // multiply by (100 * 0.0625) or 6.25
    // user defined signed value to adjust temperature measure
    tc_100 += config.temperatureAdjustment;

    whole = tc_100 / 100;  // separate off the whole and fractional portions
    fract = tc_100 % 100;

    sprintf(temperatureMsg, getProgMemMsg(TEMPERATURE_MSG_FORMAT), signBit ? '-' : '+', whole, fract < 10 ? 0 : fract);
    print(0, 0, temperatureMsg);
    if((tc_100 < config.temperatureLowThreshold) || (tc_100 > config.temperatureHighThreshold)) {
      temperatureOK = false;
    }
  }
  return temperatureOK;
}

// Return true if light is above threshold
boolean checkLight() {
  long lightLevel = 0;
  boolean lightOK = true;
  lightLevel = analogRead(LIGHT_PIN);
  sprintf(lightMsg, getProgMemMsg(LIGHT_MSG_FORMAT), lightLevel);
  print(0, 1, lightMsg);
  if(lightLevel < config.lightThreshold) {
    lightOK = false;
  }
  return(lightOK);
}

// Check for incoming SMS, and processes it if any
void checkSMS() {
  char from[PHONE_NUMBER_LENGTH + 1];
  char msgIn[20];
  char c,i;
  int cptr = 0;
  if(!gsmEnabled) return;
  Serial.println(getProgMemMsg(CHECK_SMS_MSG));
  if (sms.available())
  {
    Serial.println(getProgMemMsg(FROM_NUMBER_MSG));

    // Get remote number
    sms.remoteNumber(from, 20);
    Serial.println(from);

    // Any messages starting with # should be discarded
    if (sms.peek() == '#') {
      Serial.println(getProgMemMsg(DISCARD_SMS_MSG));
      sms.flush();
    } else {
      // Read message bytes
      while ((c = sms.read()) && (cptr < MAX_SMS_LENGTH)) {
        // If uppercase, convert to lowercase
        if ((c > 64) && (c < 91)) {
          c = c + 32;
        }
        msgIn[cptr++] = c;
      }
      msgIn[cptr] = 0;
      // Delete message from modem memory
      sms.flush();
      
      Serial.println(msgIn);
      if(strncmp(msgIn, "config", 6) == 0) {         // Sender wants to receive configuration
        sendConfig(from);
      } else if(strncmp(msgIn, "temp ", 5) == 0) {   // Sender wants to set temperture thresholds
        setTemperatureThresholds(from, msgIn);
      } else if(strncmp(msgIn, "light ", 6) == 0) {  // Sender wants to set light threshold
        setLightThreshold(from, msgIn);
      } else if(strncmp(msgIn, "save", 4) == 0) {    // Sender wants config to be saved to EEPROM
        saveConfig(from);
      } else if(strncmp(msgIn, "status", 6) == 0) {  // Sender wants to receive current measurements
        sendStatus(from);
      } else if(strncmp(msgIn, "sub ", 3) == 0) {    // Sender wants to subscribe to given service
        subscribe(from, msgIn);
      } else if(strncmp(msgIn, "unsub ",5) == 0) {   // Sender wants to unsuscrive to give service
        unsubscribe(from, msgIn);
      } else if(strncmp(msgIn, "reset sub", 9) == 0) {  // Sender wants to cancel all subscriptions
        config.registeredNumbers[0].permissionFlags = 0xFF;
        strcpy(config.registeredNumbers[0].number, REMOTE_NUMBER);
        for(i=1 ; i < MAX_PHONE_NUMBERS; i++ ) {
          config.registeredNumbers[i].permissionFlags = 0;
          config.registeredNumbers[i].number[0] = 0;
        }
      }
    }
  }
}

// Return the flag for a given service name
unsigned char getServiceFlagFromName(char *serviceName) {
  unsigned char serviceFlag = 0;

  if(0 == strncmp(serviceName, "alert", 5)) {
    serviceFlag = FLAG_SERVICE_ALERT;
  } else if(0 == strncmp(serviceName, "event", 5)) {
    serviceFlag = FLAG_SERVICE_EVENT;
  } else if(0 == strncmp(serviceName, "sub", 3)) {
    serviceFlag = FLAG_SERVICE_SUB;
  } else if(0 == strncmp(serviceName, "unsub", 5)) {
    serviceFlag = FLAG_SERVICE_UNSUB;
  }
  return serviceFlag;
}

// Set the light threshold below which an alert will be sent
void setLightThreshold(char *from, char *msgIn) {
  int threshold;
  sscanf(msgIn, "light %d", &threshold);
  config.lightThreshold = threshold;
  sendSMS(from, "Light threshold set");
}

// Set the temperatures thresholds below or above which an alert will be sent
void setTemperatureThresholds(char *from, char *msgIn) {
  int low, high;
  sscanf(msgIn, "temp %d %d", &low, &high);
  config.temperatureHighThreshold = high;
  config.temperatureLowThreshold = low;
  sendSMS(from, "Temp thresholds set");
}

// Subscribe a number to a service
void subscribe(char *number, char *msgIn) {
  unsigned char serviceFlag;
  unsigned char i, firstFree = MAX_PHONE_NUMBERS;
  boolean done = false;
  char msgBuf[70];
  char serviceName[10];

  sscanf(msgIn, "sub %s", serviceName);

  serviceFlag = getServiceFlagFromName(serviceName);
  // Check if number is already in the config
  for(i=0 ; i<MAX_PHONE_NUMBERS; i++) {
    // If current number in config is not initialized, keep its offset
    if((config.registeredNumbers[i].number[0] == 0)){
      if(firstFree == MAX_PHONE_NUMBERS) {
        firstFree = i;
      }
    } else if (0 == strncmp(config.registeredNumbers[i].number, number, PHONE_NUMBER_LENGTH)) {
      // If number found, set its flag for the given service
      config.registeredNumbers[i].permissionFlags |= serviceFlag;
      done = true;
    }
    // If number was not yet registered but a free spot is available, register it
    if(!done && (firstFree < MAX_PHONE_NUMBERS)) {
      strncpy(config.registeredNumbers[firstFree].number, number, PHONE_NUMBER_LENGTH);
      config.registeredNumbers[firstFree].permissionFlags = serviceFlag;
      done = true;
    }
  }
  if(done) {
    sprintf(msgBuf, getProgMemMsg(NUMBER_SUBSCRIBED_MSG), serviceName);
    sendSMS(number, msgBuf);
  } else {
    sprintf(msgBuf, getProgMemMsg(NUMBER_NOT_SUBSCRIBED_MSG), serviceName);
    sendSMS(number, msgBuf);
  }
}

// Unsubscribe a user from a service
void unsubscribe(char *number, char *msgIn) {
  unsigned char serviceFlag;
  unsigned char i;
  boolean done = false;
  char msgBuf[70];
  char serviceName[10];

  sscanf(msgIn, "unsub %s", serviceName);
  serviceFlag = getServiceFlagFromName(serviceName);
  // look for number
  for(i=0 ; i<MAX_PHONE_NUMBERS; i++) {
    if (0 == strncmp(config.registeredNumbers[i].number, number, PHONE_NUMBER_LENGTH)) {
      // If number found, cancel it
      config.registeredNumbers[i].permissionFlags &= (~serviceFlag);

      if(0 == config.registeredNumbers[i].permissionFlags) {
        config.registeredNumbers[i].number[0] = 0;
      }
      done = true;
    }
  }
  if(done) {
    sprintf(msgBuf, getProgMemMsg(NUMBER_UNSUBSCRIBED_MSG), serviceName);
    sendSMS(number, msgBuf);
  } else {
    sprintf(msgBuf, getProgMemMsg(NUMBER_NOT_UNSUBSCRIBED_MSG), serviceName);
    sendSMS(number, msgBuf);
  }
}

// Send SMS to all numbers subscribed to 'alerts'
void sendAlert() {
  unsigned char i, flag;
  print(0, 1, "Alerts");
  for(i=0; i < MAX_PHONE_NUMBERS; i++) {
    // If phone number initialized AND subscribed to the alert service
    if((config.registeredNumbers[i].number[0] != 0) && (0 != (config.registeredNumbers[i].permissionFlags & FLAG_SERVICE_ALERT)) ) {
      sendStatus(config.registeredNumbers[i].number);
    }
  }
}

// Send an SMS with the status to the given phone number
void sendStatus(char *toNumber) {
  char txtMsg[51];
  sprintf(txtMsg, "%s %s", temperatureMsg, lightMsg);
  txtMsg[50] = 0;
  sendSMS(toNumber, txtMsg);
}

// Send any kind of SMS to any give number
void sendSMS(char *toNumber, char *message) {
  if(toNumber[0] == 0) return;
  print(0, 0, getProgMemMsg(SENDING_SMS_MSG));
  print(0, 1, "To: ");
  print(4, 1, toNumber);
  Serial.println(message);

  // send the message
  if(gsmEnabled) {
    sms.beginSMS(toNumber);
    sms.print(message);
    sms.endSMS();
    print(0, 0, getProgMemMsg(SMS_SENT_MSG));
  }
}

// Display a message to Serial console and on the LCD display if supported
void print(int col, int row, const char* displayMsg) {
  Serial.println(displayMsg);
#if WITH_LCD_SUPPORT
  char lcdBuf[LCD_WIDTH + 1];
  int cptr = 0;
  int count = strlen(displayMsg);

  while((cptr < LCD_WIDTH) && (cptr < count)) {
    lcdBuf[cptr] = displayMsg[cptr];
    cptr++;
  }
  while(cptr < LCD_WIDTH) {
    lcdBuf[cptr] = ' ';
    cptr++;
  }
  lcdBuf[LCD_WIDTH] = 0;

  lcd.setCursor(col, row);
  lcd.print(lcdBuf);
#endif
}

#if WITH_IR_SUPPORT
// Remove the IR indicators from the LCD display
void resetIRDisplay() {
#if WITH_LCD_SUPPORT
  //lcd.setCursor(14,0);
  //lcd.print("  ");
  lcd.setCursor(15,1);
  lcd.print(" ");
#endif
}

// Check if an IR code was received and process it according to its value
void processIRCode(unsigned long code) {
  char msgBuf[70];
  if((irCode == IR_REPEAT_CODE) && (0 != previousCode)) {
    irCode = previousCode;
  }
  previousCode = irCode;
  lastIrDisplay = millis();
  Serial.println(irCode, HEX);
  //print(14, 0, "ir");
  switch(irCode) {
    case IR_DISPLAY_PHONE_NUMBER:
      // TOTO loop over config
    break;
    case IR_INCR_LIGHT_THRESHOLD:
      config.lightThreshold ++;
      sprintf(msgBuf, getProgMemMsg(LIGHT_THRESHOLD_MSG_FORMAT), config.lightThreshold);
      print(0, 1, msgBuf);
    break;
    case IR_DECR_LIGHT_THRESHOLD:
      config.lightThreshold --;
      sprintf(msgBuf, getProgMemMsg(LIGHT_THRESHOLD_MSG_FORMAT), config.lightThreshold);
      print(0, 1, msgBuf);
    break;

    case IR_INCR_TEMP_ADJUSTMENT:
      config.temperatureAdjustment ++;
      sprintf(msgBuf, getProgMemMsg(TEMPERATURE_ADJUSTMENT_MSG_FORMAT), config.temperatureAdjustment);
      print(0, 1, msgBuf);
    break;
    case IR_DECR_TEMP_ADJUSTMENT:
      config.temperatureAdjustment --;
      sprintf(msgBuf, getProgMemMsg(TEMPERATURE_ADJUSTMENT_MSG_FORMAT), config.temperatureAdjustment);
      print(0, 1, msgBuf);
    break;

    case IR_DISPLAY_THRESHOLDS:
      sprintf(msgBuf, getProgMemMsg(TEMPERATURE_THRESHOLD_MSG_FORMAT), config.temperatureLowThreshold, config.temperatureHighThreshold);
      print(0, 0, msgBuf);
      sprintf(msgBuf, getProgMemMsg(LIGHT_THRESHOLD_MSG_FORMAT), config.lightThreshold);
      print(0, 1, msgBuf);
    break;

    case IR_DISPLAY_TEMP_ADJUST:
      sprintf(msgBuf, getProgMemMsg(TEMPERATURE_ADJUSTMENT_MSG_FORMAT), config.temperatureAdjustment);
      print(0, 0, msgBuf);
    break;

    case IR_SAVE_TO_EEPROM:
      saveConfig("");
    break;
    case IR_LOG_CONFIG:
      logConfig();
    break;
    default:
      print(15, 1, "?");
  }
}

// Log config to Serial console, on request from IR remote control
void logConfig() {
  char i;
  for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
    Serial.print(i);
    Serial.print(" ");
    Serial.print(config.registeredNumbers[i].number);
    Serial.print(" ");
    Serial.println(config.registeredNumbers[i].permissionFlags);
  }
}
#endif

// Send SMS with all configuration. Only if requesting number has the 'admin' flag set in config
void sendConfig(char *toNumber) {
  char i;
  char message[40];

  if(!checkAdmin(toNumber)) return;

  sms.beginSMS(toNumber);
  sprintf(message, getProgMemMsg(LIGHT_THRESHOLD_MSG_FORMAT), config.lightThreshold);
  sms.print(message);
  sms.print(", ");
  sprintf(message, getProgMemMsg(TEMPERATURE_THRESHOLD_MSG_FORMAT), config.temperatureLowThreshold, config.temperatureHighThreshold);
  sms.print(message);
  sms.print(", ");
  sprintf(message, getProgMemMsg(TEMPERATURE_ADJUSTMENT_MSG_FORMAT), config.temperatureAdjustment);
  sms.print(message);
  sms.print(", ");
  for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
    if(config.registeredNumbers[i].number[0] != 0) {
      sprintf(message, "%s %2X,", config.registeredNumbers[i].number, config.registeredNumbers[i].permissionFlags);
      sms.print(message);
    }
  }
  sms.endSMS();
}

// Return true/false depending on phone number having the admin flag
boolean checkAdmin(char *number) {
  boolean isAdmin = false;
  char i;
  for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
    if(strncmp(config.registeredNumbers[i].number, number, PHONE_NUMBER_LENGTH)) {
      if(config.registeredNumbers[i].permissionFlags & FLAG_ADMIN) {
        isAdmin = true;
        break;
      }
    }
  }
  return(isAdmin);
}

// Init config structure from EEPROM
// If not in EEPROM or version has changed from what is stored in EEPROM, 
// reset config to default values, and save it to EEPROM for next time
void readConfig() {
  unsigned char i;
  print(0, 1, getProgMemMsg(READING_CONFIG_MSG));
  EEPROM.readBlock(CONFIG_ADDRESS, config);
  if(config.version != CONFIG_VERSION) {
    config.version = CONFIG_VERSION;
    config.temperatureAdjustment = 0;
    config.temperatureHighThreshold = 2700;
    config.temperatureLowThreshold = 2100;
    config.lightThreshold = 200;
    config.lightOnHour = 13;
    config.lightOnMinute = 30;
    config.lightOffHour = 20;
    config.lightOffMinute = 30;

    // Reset all saved numbers
    // First one is main number, hardcoded, admin and subscribed to all services
    for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
      if(i == 0) {
        config.registeredNumbers[i].permissionFlags = 0xFF;
        strcpy(config.registeredNumbers[i].number, REMOTE_NUMBER);
      } else {
        config.registeredNumbers[i].permissionFlags = 0;
        config.registeredNumbers[i].number[0] = 0;
      }
      config.registeredNumbers[i].lastAlertSmsTime = 0;
      config.registeredNumbers[i].minAlertInterval = 60 * 30; // Every 30 minutes = 1800 seconds
    }

    // Should we automatically save ? or wait for a save request ?
    saveConfig("");
  }
}

// Save the configuration to EEPROM
// TODO : should that be for admin only ?
void saveConfig(char *toNumber) {
  print(0, 0, getProgMemMsg(SAVING_CONFIG_MSG));
  EEPROM.writeBlock(CONFIG_ADDRESS, config);
  sendSMS(toNumber, getProgMemMsg(CONFIG_SAVED_MSG));
}


