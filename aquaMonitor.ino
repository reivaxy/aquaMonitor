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
// This file should contain a line in the likes of:
// #define ADMIN_NUMBER "+12345678910", which is the international number notation
#include <remoteNumber.h>

// Compilation directives to enable/disable stuff like IR support
// Beware, 'includes' for the matching libraries need to be commented / uncommented
// In the first #if using these flags
#define WITH_LCD_SUPPORT true      // About 1,6k prog, 30B RAM

// Strings to store in progmem space to free variable space
#include "./progmemStrings.h"

#define PHONE_NUMBER_LENGTH 15
#define MAX_PHONE_NUMBERS 4

#define DEFAULT_MIN_ALERT_INTERVAL 1800000 // Half an hour
struct phoneConfig {
  unsigned char permissionFlags;
  char number[PHONE_NUMBER_LENGTH + 1];
  unsigned long lastAlertSmsTime = 0;  // When was last time (millis()) an alert SMS sent to that number
  unsigned long minAlertInterval = DEFAULT_MIN_ALERT_INTERVAL;  // minimum interval in milliseconds between 2 alert SMS to avoir flooding
};

// Change this version to reset the EEPROM saved configuration and/or to init date and time
// when the structure changes
#define CONFIG_VERSION 10
#define CONFIG_ADDRESS 16        // Do not use adress 0, it is not reliable.
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
  int highLevelPinValue;         // Value (HIGH or LOW) on level pin when water level is ok
  phoneConfig registeredNumbers[MAX_PHONE_NUMBERS];  // Phone numbers to send alerts to, according to their subscriptions and permissions
} config;

struct displayData {
  char temperatureMsg[20];
  char lightMsg[20];
  char levelMsg[20];
  unsigned char offset = 0;
  // TODO size below should somehow be LCD width, but what if LCD disabled with compilation directives ?
  char permanent[17]; // Message always displayed
  // Transient message will be displayed for short durations
  unsigned long transientStartDisplayTime=0;
  unsigned long transientDisplayDuration=2000; // 2 seconds
  unsigned long refreshPeriod = 400; // every 100 ms
  unsigned long lastRefresh = 0;
} display;

// Flags for services subscriptions and permissions
#define FLAG_SERVICE_ALERT   0x01
#define FLAG_SERVICE_EVENT   0x02

#define FLAG_ADMIN           0x80

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
// GMS uses 2 and 3 (at least)
LiquidCrystal lcd(4,5,6,8,9,13);
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#endif

#define LIGHT_PIN 0
#define TEMPERATURE_PIN 12

/* DS18B20 Temperature chip i/o */
OneWire  ds(TEMPERATURE_PIN);
#define MAX_DS1820_SENSORS 1
byte addr[MAX_DS1820_SENSORS][8];

// Okay these globals are pretty bad and still the cause of a few bugs, but the
// small amount of variable space left me with little choice.
#define PROGMEM_MSG_MAX_SIZE 40
char progMemMsg[PROGMEM_MSG_MAX_SIZE + 1];

#define TEMPERATURE_CHECK_PERIOD 5000
unsigned long lastTemperatureCheck = millis();

#define LIGHT_CHECK_PERIOD 5000
unsigned long lastLightCheck = millis();

#define SERIAL_STATUS_PERIOD 5000
unsigned long lastSerialStatus = millis();

#define LEVEL_CHECK_PERIOD 5000
unsigned long lastLevelCheck = millis();

// Check for incoming sms every 30 seconds
#define SMS_CHECK_PERIOD 30000
unsigned long lastSmsCheck = millis();

// Max size of a received SMS
#define MAX_SMS_LENGTH 30
boolean gsmEnabled = !false;

boolean statusOK = true;

#define LEVEL_PIN 11

// TODO: offer choice with DS1302 ?
DS1307 clock; // The RTC handle to get date and time

// System initialization
void setup(void) {
  Serial.begin(9600);

  // Init pin with the level detector as input
  pinMode(LEVEL_PIN, INPUT);
  digitalWrite(LEVEL_PIN, HIGH); // Activate pullup resistor

  // Just in case some loop goes crazy, limit the number of EEPROM writes to save it
  // TODO : this should be removed some day
  EEPROM.setMaxAllowedWrites(100);

  // Initialise RTC
  clock.begin();

  initLCD();

  displayTransient(getProgMemMsg(INIT_AQUAMON_MSG));
  Serial.println(getProgMemMsg(BUILD_MSG));

  readConfig();
  displayConfig();

  displayTransient(getProgMemMsg(TEMP_INIT_MSG));
  if (!ds.search(addr[0])) {
    displayTransient(getProgMemMsg(ADDR_ERR_MSG));
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
  displayTransient(getProgMemMsg(INIT_GSM_MSG));
  while(notConnected) {
    displayTransient(getProgMemMsg(CONNECTING_GSM_MSG));
    if(gsmAccess.begin(PINNUMBER)==GSM_READY) {
      displayTransient(getProgMemMsg(CONNECTED_GSM_MSG));
      notConnected = false;
    } else {
      displayTransient(getProgMemMsg(NOT_CONNECTED_GSM_MSG));
      delay(200);
    }
  }
  delay(500);
  // Not working... TODO investigate. Delay ?
  // sendSMS(config.registeredNumbers[0].number, getProgMemMsg(BUILD_MSG));
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

  if(checkElapsedDelay(now, display.lastRefresh, display.refreshPeriod)) {
    refreshDisplay();
  }

  if(checkElapsedDelay(now, lastLightCheck, LIGHT_CHECK_PERIOD)) {
    statusOK = checkLight() && statusOK;
    lastLightCheck = now;
  }
  if(checkElapsedDelay(now, lastTemperatureCheck, TEMPERATURE_CHECK_PERIOD)) {
    statusOK = checkTemperature() && statusOK;
    lastTemperatureCheck = millis();
  }
  if(checkElapsedDelay(now, lastLevelCheck, LEVEL_CHECK_PERIOD)) {
    statusOK = checkLevel() && statusOK;
    lastLevelCheck = now;
  }
  if(checkElapsedDelay(now, lastSmsCheck, SMS_CHECK_PERIOD)) {
    checkSMS();
    lastSmsCheck = millis(); // checkSMS is a bit slow.
  }

  if(!statusOK) {
    sendAlert();
    statusOK = true;
  }
}

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

boolean checkLevel() {
  boolean levelOK = true;
  levelOK = (config.highLevelPinValue == digitalRead(LEVEL_PIN));
  if(levelOK) {
    sprintf(display.levelMsg, getProgMemMsg(LEVEL_HIGH_MSG));
    deletePermanent(getProgMemMsg(LEVEL_ALERT_MSG));
  } else {
    sprintf(display.levelMsg, getProgMemMsg(LEVEL_LOW_MSG));
    displayPermanent(getProgMemMsg(LEVEL_ALERT_MSG));
  }
  return(levelOK);
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
    displayTransient(getProgMemMsg(CRC_NOT_VALID_MSG));
  } else if ( addr[sensor][0] != 0x28) {
    displayTransient(getProgMemMsg(FAMILY_MSG));
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
    sprintf(display.temperatureMsg, getProgMemMsg(TEMPERATURE_MSG_FORMAT), signBit ? '-' : '+', whole, fract < 10 ? 0 : fract);
    //Serial.println(display.temperatureMsg);
    if((tc_100 < config.temperatureLowThreshold) || (tc_100 > config.temperatureHighThreshold)) {
      temperatureOK = false;
      displayPermanent(getProgMemMsg(TEMPERATURE_ALERT_MSG));
    } else {
      deletePermanent(getProgMemMsg(TEMPERATURE_ALERT_MSG));
    }
  }
  return temperatureOK;
}

// Return true if light is below threshold within scheduled time frame
boolean checkLight() {
  long lightLevel = 0;
  boolean lightOK = true;
  lightLevel = analogRead(LIGHT_PIN);
  sprintf(display.lightMsg, getProgMemMsg(LIGHT_MSG_FORMAT), lightLevel);
  // If light level less than threshold during light on period => not ok
  if((lightLevel < config.lightThreshold) && inLightSchedule()) {
    lightOK = false;
    displayPermanent(getProgMemMsg(LIGHT_ALERT_MSG));
  } else {
    deletePermanent(getProgMemMsg(LIGHT_ALERT_MSG));
  }
  return(lightOK);
}

// check if now is within the schedule during which light should be on
boolean inLightSchedule() {
  boolean in = false;
  unsigned long onMin;
  unsigned long offMin;
  unsigned long nowMin;

  clock.getTime();
  onMin = config.lightOnHour * 60 + config.lightOnMinute;
  offMin = config.lightOffHour * 60 + config.lightOffMinute;
  nowMin = clock.hour * 60 + clock.minute;
  if((nowMin >= onMin) && (nowMin <= offMin)) {
    in = true;
  }
  return(in);
}

// Check for incoming SMS, and processes it if any
void checkSMS() {
  char from[PHONE_NUMBER_LENGTH + 1];
  char msgIn[MAX_SMS_LENGTH + 1];
  char c,i;
  int cptr = 0;
  if(!gsmEnabled) {
    Serial.println('NO GSM');
    return;
  }
  displayTransient(getProgMemMsg(CHECK_SMS_MSG));
  // TODO : loop to process all incoming SMS
  while (sms.available())
  {
    Serial.println(getProgMemMsg(FROM_NUMBER_MSG));
    cptr = 0;
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
      if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_INTERVAL))) {   // Sender wants to set his alert minimum interval (seconds)
        setAlertInterval(from, msgIn);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_TEMP_ADJ))) {  // Sender wants to set temperature adjustment
        setTemperatureAdjustment(from, msgIn);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SUBS))) {   // Sender wants to receive subscription information
        sendSubs(from);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_CONFIG))) {   // Sender wants to receive configuration
        sendConfig(from);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_TEMP))) {   // Sender wants to set temperture thresholds
        setTemperatureThresholds(from, msgIn);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_LIGHT))) {  // Sender wants to set light threshold
        setLightThreshold(from, msgIn);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SCHEDULE))) {  // Sender wants to set light schedule
        setLightSchedule(from, msgIn);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SAVE))) {    // Sender wants config to be saved to EEPROM
        saveConfig(from);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_STATUS))) {  // Sender wants to receive current measurements
        sendStatus(from);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SUB))) {    // Sender wants to subscribe to given service
        subscribe(from, msgIn);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_UNSUB))) {   // Sender wants to unsubscribe to give service
        unsubscribe(from, msgIn);
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_RESET_SUB))) {  // Sender wants to cancel all subscriptions
        if(!checkAdmin(from)) {
          sendSMS(from, getProgMemMsg(ACCESS_DENIED_MSG));
        } else {
          resetSub();
          sendSMS(from, getProgMemMsg(RESET_SUB_DONE_MSG));
        }
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_RESET_LCD))) {  // Sender wants to reset the displa
        initLCD();
      } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SET_ADMIN))) {  // Sender wants to change admin phone number
        if(!checkAdmin(from)) {
          sendSMS(from, getProgMemMsg(ACCESS_DENIED_MSG));
        } else {
          setAdmin(from, msgIn);
        }
      } else {
        displayTransient(getProgMemMsg(UNKNOWN_MSG));
        sendSMS(from, getProgMemMsg(UNKNOWN_MSG));
      }
    }
  }
}

// Return the flag for a given service name
unsigned char getServiceFlagFromName(char *serviceName) {
  unsigned char serviceFlag = 0;

  if(0 == strncmp(serviceName, "alert", 5)) {
    serviceFlag = FLAG_SERVICE_ALERT;
  } else if(0 == strncmp(serviceName, "event", 5)) {  // Could be light on, off
    serviceFlag = FLAG_SERVICE_EVENT;
  }
  // More flags ?
  return serviceFlag;
}

// Search registered number for one given number.
// If found, store its offset in foundOrFree and return true
// If not, return false with the first available entry offset in foundOrFree
// If no available, set foundOrFree to -1

boolean findRegisteredNumber(char *number, char *foundOrFree) {
  unsigned char i, firstFree = MAX_PHONE_NUMBERS;
  boolean result = false;
  *foundOrFree = -1;
  for(i=0 ; i<MAX_PHONE_NUMBERS; i++) {
    if((config.registeredNumbers[i].number[0] == 0)) {
      if(*foundOrFree == -1) {
        // First available position to store a number
        *foundOrFree = i;
      }
    } else if (0 == strncmp(config.registeredNumbers[i].number, number, PHONE_NUMBER_LENGTH)) {
      // If number found, set its flag for the given service
      *foundOrFree = i;
      result = true;
      break;
    }
  }
  return result;
}

// Set the light threshold below which an alert will be sent
void setLightThreshold(char *from, char *msgIn) {
  int threshold;
  sscanf(msgIn, getProgMemMsg(IN_SMS_LIGHT_FORMAT), &threshold);
  config.lightThreshold = threshold;
  sendSMS(from, getProgMemMsg(LIGHT_THRESHOLD_SET_MSG));
}

// Admin can change the admin number
void setAdmin(char *from, char *msgIn) {
  char newAdmin[PHONE_NUMBER_LENGTH + 1];
  sscanf(msgIn, getProgMemMsg(IN_SMS_SET_ADMIN_FORMAT), newAdmin); // not really safe
  newAdmin[PHONE_NUMBER_LENGTH] = 0;  // Make sure...
  subscribeFlag(newAdmin, FLAG_ADMIN | FLAG_SERVICE_ALERT);
  sendSMS(from, getProgMemMsg(SET_ADMIN_DONE_MSG));
}

// Set the light threshold below which an alert will be sent
void setLightSchedule(char *from, char *msgIn) {
  unsigned int hourOn, minuteOn, hourOff, minuteOff;
  sscanf(msgIn, getProgMemMsg(IN_SMS_SCHEDULE_FORMAT), &hourOn, &minuteOn, &hourOff, &minuteOff);
  config.lightOnHour = (byte)hourOn;
  config.lightOnMinute = (byte)minuteOn;
  config.lightOffHour = (byte)hourOff;
  config.lightOffMinute = (byte)minuteOff;
  sendSMS(from, getProgMemMsg(LIGHT_SCHEDULE_SET_MSG));
}

// Set the minimum alert interval to not flood a number with alert SMS
void setAlertInterval(char *from, char *msgIn) {
  unsigned long interval;
  char offset;
  boolean found = false;
  found = findRegisteredNumber(from, &offset);
  if(found) {
    sscanf(msgIn, getProgMemMsg(IN_SMS_INTERVAL_FORMAT), &interval);
    Serial.println(interval);
    config.registeredNumbers[offset].minAlertInterval = interval * 1000UL;   // Given in seconds, stored in milliseconds
    Serial.println(getProgMemMsg(INTERVAL_SET_MSG));
    displayConfig();
    sendSMS(from, getProgMemMsg(INTERVAL_SET_MSG));
  } else {
    Serial.println(getProgMemMsg(UNKNOWN_NUMBER_MSG));
  }
}

// Set the temperatures thresholds below or above which an alert will be sent
void setTemperatureThresholds(char *from, char *msgIn) {
  int low, high;
  sscanf(msgIn, getProgMemMsg(IN_SMS_TEMP_FORMAT), &low, &high);
  config.temperatureHighThreshold = high;
  config.temperatureLowThreshold = low;
  sendSMS(from, getProgMemMsg(TEMPERATURE_THRESHOLDS_SET_MSG));
}

// Set the temperature adjustment parameter
void setTemperatureAdjustment(char *from, char *msgIn) {
  int adj;
  sscanf(msgIn, getProgMemMsg(IN_SMS_TEMP_ADJ_FORMAT), &adj);
  config.temperatureAdjustment = adj;
  sendSMS(from, getProgMemMsg(TEMPERATURE_ADJUSTMENT_SET_MSG));
}

// Subscribe a number to a service
void subscribe(char *number, char *msgIn) {
  unsigned char serviceFlag;
  unsigned char i, firstFree = MAX_PHONE_NUMBERS;
  boolean done = false;
  char msgBuf[30];
  char serviceName[10];

  sscanf(msgIn, getProgMemMsg(IN_SMS_SUB_FORMAT), serviceName);
  serviceFlag = getServiceFlagFromName(serviceName);
  if(serviceFlag != 0) {
    done = subscribeFlag(number, serviceFlag);
  }

  if(done) {
    sprintf(msgBuf, getProgMemMsg(NUMBER_SUBSCRIBED_MSG), serviceName);
    sendSMS(number, msgBuf);
  } else {
    sprintf(msgBuf, getProgMemMsg(NUMBER_NOT_SUBSCRIBED_MSG), serviceName);
    sendSMS(number, msgBuf);
  }
}

boolean subscribeFlag(char *number, unsigned char flag) {
  boolean done = false;
  boolean found = false;
  char foundOrFree;

  // Check if number is already in the config
  found = findRegisteredNumber(number, &foundOrFree);
  if(found) {
    // foundOrFree is offset of found number
    config.registeredNumbers[foundOrFree].permissionFlags |= flag;
    done = true;
  } else {
    // foundOrfree is first offset free, or -1 if no more room
    if(foundOrFree != -1) {
      strncpy(config.registeredNumbers[foundOrFree].number, number, PHONE_NUMBER_LENGTH);
      config.registeredNumbers[foundOrFree].permissionFlags = flag;
      config.registeredNumbers[foundOrFree].lastAlertSmsTime = millis();
      config.registeredNumbers[foundOrFree].minAlertInterval = DEFAULT_MIN_ALERT_INTERVAL;
      done = true;
    }
  }
  return done;
}

// Unsubscribe a user from a service
void unsubscribe(char *number, char *msgIn) {
  unsigned char serviceFlag;
  unsigned char i;
  char msgBuf[30];
  char serviceName[10];
  boolean found = false;
  char foundOrFree;

  sscanf(msgIn, getProgMemMsg(IN_SMS_UNSUB_FORMAT), serviceName);
  serviceFlag = getServiceFlagFromName(serviceName);
  // look for number
  found = findRegisteredNumber(number, &foundOrFree);
  if(found) {
    config.registeredNumbers[foundOrFree].permissionFlags &= (~serviceFlag);
    // If no more flags, remove number
    if(0 == config.registeredNumbers[foundOrFree].permissionFlags) {
      config.registeredNumbers[foundOrFree].number[0] = 0;
    }
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
  unsigned long now = millis();  // We don't want to send too many SMS
  char txtMsg[51];

  for(i=0; i < MAX_PHONE_NUMBERS; i++) {
    // If phone number initialized AND subscribed to the alert service
    if((config.registeredNumbers[i].number[0] != 0) && (0 != (config.registeredNumbers[i].permissionFlags & FLAG_SERVICE_ALERT)) ) {
      // If enough time since last alert SMS was sent to this number, send a new one
      if(checkElapsedDelay(now, config.registeredNumbers[i].lastAlertSmsTime, config.registeredNumbers[i].minAlertInterval)) {
        sprintf(txtMsg, "ALERT %s %s %s", display.temperatureMsg, display.lightMsg, display.levelMsg);
        txtMsg[50] = 0; // just in case
        sendSMS(config.registeredNumbers[i].number, txtMsg);
        config.registeredNumbers[i].lastAlertSmsTime = now;
      }
    }
  }
}

// Send an SMS with the status to the given phone number
void sendStatus(char *toNumber) {
  char txtMsg[51];
  sprintf(txtMsg, "%s %s %s", display.temperatureMsg, display.lightMsg, display.levelMsg);
  txtMsg[50] = 0; // just in case
  sendSMS(toNumber, txtMsg);
}

// Send any kind of SMS to any give number
void sendSMS(char *toNumber, char *message) {
  char msg[200];
  // If message comes from progmem, it's in a global :(
  // that will be re used a few lines below
  strncpy(msg, message, 199);
  msg[199] = 0;
  if(toNumber[0] == 0) return;
  Serial.println(msg);

  // send the message
  if(gsmEnabled) {
    displayTransient(getProgMemMsg(SENDING_SMS_MSG));
    sms.beginSMS(toNumber);
    sms.print(msg);
    sms.endSMS();
    displayTransient(getProgMemMsg(SMS_SENT_MSG));
  }
}

// Log config to Serial console
void displayConfig() {
  unsigned char i;
  char message[50];
  char strFlags[5];
  clock.getTime();
  sprintf(message, getProgMemMsg(CURRENT_DATE_FORMAT_MSG),
     clock.year+2000, clock.month, clock.dayOfMonth,
     clock.hour, clock.minute);
  Serial.println(message);

  sprintf(message, getProgMemMsg(LIGHT_THRESHOLD_MSG_FORMAT), config.lightThreshold);
  Serial.println(message);

  sprintf(message, getProgMemMsg(LIGHT_SCHEDULE_MSG_FORMAT),
                config.lightOnHour, config.lightOnMinute,
                config.lightOffHour, config.lightOffMinute);
  Serial.println(message);

  sprintf(message, getProgMemMsg(TEMPERATURE_THRESHOLD_MSG_FORMAT), config.temperatureLowThreshold, config.temperatureHighThreshold);
  Serial.println(message);
  sprintf(message, getProgMemMsg(TEMPERATURE_ADJUSTMENT_MSG_FORMAT), config.temperatureAdjustment);
  Serial.println(message);

  for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
    if(config.registeredNumbers[i].number[0] != 0) {
      Serial.print(i);
      Serial.print(" ");
      Serial.print(config.registeredNumbers[i].number);
      Serial.print(" ");
      sprintf(strFlags, "%2X", config.registeredNumbers[i].permissionFlags);
      Serial.print(strFlags);
      Serial.print(" ");
      Serial.println(config.registeredNumbers[i].minAlertInterval);
    }
  }
}


// Send SMS with all configuration. Only if requesting number has the 'admin' flag set in config
void sendConfig(char *toNumber) {
  char message[40];
  char space[] = ", ";

  displayConfig(); // Display through serial
  if(gsmEnabled) {

    sms.beginSMS(toNumber);
    sprintf(message, getProgMemMsg(BUILD_MSG));
    sms.print(message);
    sms.print(space);

    clock.getTime();
    sprintf(message, getProgMemMsg(CURRENT_DATE_FORMAT_MSG),
       clock.year+2000, clock.month, clock.dayOfMonth,
       clock.hour, clock.minute);
    sms.print(message);
    sms.print(space);

    sprintf(message, getProgMemMsg(LIGHT_THRESHOLD_MSG_FORMAT), config.lightThreshold);
    sms.print(message);
    sms.print(space);

    sprintf(message, getProgMemMsg(LIGHT_SCHEDULE_MSG_FORMAT),
                  config.lightOnHour, config.lightOnMinute,
                  config.lightOffHour, config.lightOffMinute);
    sms.print(message);
    sms.print(space);

    sprintf(message, getProgMemMsg(TEMPERATURE_THRESHOLD_MSG_FORMAT), config.temperatureLowThreshold, config.temperatureHighThreshold);
    sms.print(message);
    sms.print(space);
    sprintf(message, getProgMemMsg(TEMPERATURE_ADJUSTMENT_MSG_FORMAT), config.temperatureAdjustment);
    sms.print(message);
    // Message needs to be less than 160c
    sms.endSMS();
  }
}

// Send a message listing all subscriptions
void sendSubs(char *toNumber) {
  char i;
  char message[30];
  unsigned long now = millis();
  if(!checkAdmin(toNumber)) {
    sendSMS(toNumber, getProgMemMsg(ACCESS_DENIED_MSG));
    return;
  }
  //displayConfig(); // Display through serial

  if(gsmEnabled) {
    sms.beginSMS(toNumber);
    for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
      if(config.registeredNumbers[i].number[0] != 0) {
        sprintf(message, "%s %2X %d %d ,", config.registeredNumbers[i].number,
         config.registeredNumbers[i].permissionFlags, config.registeredNumbers[i].minAlertInterval / 1000, 
         (now - config.registeredNumbers[i].lastAlertSmsTime) / 1000
         );
        sms.print(message);
      }
    }
    sms.endSMS();
  }
}

// Return true/false depending on phone number having the admin flag
boolean checkAdmin(char *number) {
  boolean isAdmin = false;
  char i;
  for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
    if(0 == strncmp(config.registeredNumbers[i].number, number, PHONE_NUMBER_LENGTH)) {
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
  EEPROM.readBlock(CONFIG_ADDRESS, config);
  if(config.version != CONFIG_VERSION) {
    displayTransient(getProgMemMsg(NEW_CONFIG_MSG));

    config.version = CONFIG_VERSION;
    config.temperatureAdjustment = 0;
    config.temperatureHighThreshold = 2700;
    config.temperatureLowThreshold = 2100;
    config.lightThreshold = 200;
    config.lightOnHour = 13;
    config.lightOnMinute = 30;
    config.lightOffHour = 20;
    config.lightOffMinute = 30;
    config.highLevelPinValue = HIGH;

    // Reset all subscriptions
    resetSub();
    // Should we automatically save ? or wait for a save request ?
    saveConfig("");
    setupClock();
  }
}

// reset all subscriptions and admin number
void resetSub() {
  char i;
  unsigned long now = millis();
  for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
    if(i == 0) {
      // admin is subscribed to all services
      config.registeredNumbers[i].permissionFlags = 0x8F;
      strcpy(config.registeredNumbers[i].number, ADMIN_NUMBER);
    } else {
      config.registeredNumbers[i].permissionFlags = 0;
      config.registeredNumbers[i].number[0] = 0;
    }
    config.registeredNumbers[i].minAlertInterval = 1800000; // Every 30 minutes = 1800 seconds
    config.registeredNumbers[i].lastAlertSmsTime = 0;
  }
  displayConfig();
}

// Save the configuration to EEPROM
// TODO : should that be for admin only ?
void saveConfig(char *toNumber) {
  displayTransient(getProgMemMsg(SAVING_CONFIG_MSG));
  EEPROM.writeBlock(CONFIG_ADDRESS, config);
  sendSMS(toNumber, getProgMemMsg(CONFIG_SAVED_MSG));
}

// In charge of displaying messages
void refreshDisplay() {
  char message[100];
  char scrolledMessage[100];
  char transferChar;
  int remaining;
  unsigned long now = millis();

  display.lastRefresh = millis();
  if(checkElapsedDelay(now, lastSerialStatus, SERIAL_STATUS_PERIOD)) {
    Serial.print(display.temperatureMsg);
    Serial.print(" ");
    Serial.print(display.lightMsg);
    Serial.print(" ");
    Serial.println(display.levelMsg);
    lastSerialStatus = now;
  }

  sprintf(message, "%s, %s, %s, ", display.temperatureMsg, display.lightMsg, display.levelMsg);
  remaining = strlen(message) - display.offset;
  strncpy(scrolledMessage, message + display.offset, 16);
  strncat(scrolledMessage, message, display.offset );
  scrolledMessage[16] = 0;
  display.offset++;
  if(remaining == 0) {
    display.offset = 0;
  }
  #if WITH_LCD_SUPPORT
  if(checkElapsedDelay(millis(), display.transientStartDisplayTime, display.transientDisplayDuration)) {
    lcd.clear();
    displayPermanent(NULL);
  }
  lcd.setCursor(0,0);
  lcd.print(scrolledMessage);
  #endif
}

void displayTransient(char *msg) {
  #if WITH_LCD_SUPPORT
  lcd.setCursor(0,1);
  while(strlen(msg) < 16) {
    strcat(msg, " ");
  }
  lcd.print(msg);
  display.transientStartDisplayTime = millis();
  #endif
  Serial.println(msg);
}

void displayPermanent(char *msg) {
  if(msg != NULL) {
    strncpy(display.permanent, msg, 16);
    display.permanent[16] = 0;
  }
#if WITH_LCD_SUPPORT
  lcd.setCursor(0,1);
  lcd.print(display.permanent);
#endif
}

void deletePermanent(char *msg) {
  if(0 == strncmp(display.permanent, msg, 16))  {
#if WITH_LCD_SUPPORT
    lcd.clear();
#endif
    display.permanent[0] = 0;
  }
}

void initLCD() {
#if WITH_LCD_SUPPORT
  lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
  lcd.clear();
#endif
}

void setupClock()
{
  int hour, min, sec, day, month, year;
  char monthStr[5];

  sscanf(__DATE__, "%s %d %d", monthStr, &day, &year);
  sscanf(__TIME__, "%d:%d:%d", &hour, &min, &sec);
  clock.getTime();
  if(strncmp(monthStr, "Jan", 3) == 0)
    month = 1;
  if(strncmp(monthStr, "Feb", 3) == 0)
    month = 2;
  if(strncmp(monthStr, "Mar", 3) == 0)
    month = 3;
  if(strncmp(monthStr, "Apr", 3) == 0)
    month = 4;
  if(strncmp(monthStr, "May", 3) == 0)
    month = 5;
  if(strncmp(monthStr, "Jun", 3) == 0)
    month = 6;
  if(strncmp(monthStr, "Jul", 3) == 0)
    month = 7;
  if(strncmp(monthStr, "Aug", 3) == 0)
    month = 8;
  if(strncmp(monthStr, "Sep", 3) == 0)
    month = 9;
  if(strncmp(monthStr, "Oct", 3) == 0)
    month = 10;
  if(strncmp(monthStr, "Nov", 3) == 0)
    month = 11;
  if(strncmp(monthStr, "Dec", 3) == 0)
    month = 12;
  // Run this only once
  Serial.println("Setting time");
  Serial.println(__DATE__);
  Serial.println(__TIME__);
  clock.fillByYMD(year, month, day);
  clock.fillByHMS(hour, min, sec + 8);  // +8 for linking/uploading offset ;) Very experimental :)
  clock.setTime();
}

