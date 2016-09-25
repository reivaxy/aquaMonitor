
#include <OneWire.h>
#include <GSM.h>
#include <EEPROMex.h>
#include <EEPROMvar.h>
#include <Wire.h>
#include "DS1307.h"
#include "aquaNet/interComMsg.h"
#include "aquaNet/common.h"
#include <ArduinoJson.h>

// 'admin' phone number defined outside of open source file haha !
// This file should contain a line in the likes of:
// #define ADMIN_NUMBER "+12345678910", which is the international number notation
#include "aquaMonitorSecret.h"

// Compilation directives to enable/disable stuff like IR support
// Beware, 'includes' for the matching libraries need to be commented / uncommented
// In the first #if using these flags
#define WITH_LCD_SUPPORT false      // About 1,6k prog, 30B RAM  TSTWIFI

// Strings to store in progmem space to free variable space
#include "./progmemStrings.h"

// Two light schedules will be handled: one during which lights must be on, one during which lights must be off
// For each, min and max level will specify the ok value for the light sensor.
// If value is not within these limits during the specified interval, alert will be sent
struct lightSchedule {
  byte startHour;           // Hour of the begining of the interval
  byte startMinute;         // Minute of the begining of the interval
  byte endHour;             // Hour of the ending of the interval
  byte endMinute;           // Minute of the ending of the interval
  int minAcceptableValue;   // Min value for light sensor considered ok for this schedule
  int maxAcceptableValue;   // Max value for light sensor considered ok for this schedule
};

// If not declaring this prototype explicitly before calling the function, arduino compiler
// will create one *before* the definition of lightSchedule and issue a crappy error message
// because the parameter will not have the right type.
boolean checkLightSchedule(int lightLevel, lightSchedule schedule);

#define PHONE_NUMBER_LENGTH 15
#define MAX_PHONE_NUMBERS 4

#define DEFAULT_MIN_ALERT_INTERVAL 1800000 // Half an hour
struct phoneConfig {
  byte permissionFlags;
  char number[PHONE_NUMBER_LENGTH + 1];
  unsigned long lastAlertSmsTime = 0UL;  // When was last time (millis()) an alert SMS sent to that number
  unsigned long minAlertInterval = DEFAULT_MIN_ALERT_INTERVAL;  // minimum interval in milliseconds between 2 alert SMS to avoid flooding
};

// Change this version to reset the EEPROM saved configuration and/or to init date and time
// when the structure changes
#define CONFIG_VERSION 11
#define CONFIG_ADDRESS 16        // Do not use adress 0, it is not reliable (read somewhere).
struct eepromConfig {
  unsigned int version;          // Version for this config structure and default values. Always keep as first structure member 
  int temperatureAdjustment;     // Signed offset to add to temperature measure to adjust it
  int temperatureHighThreshold;  // Alert will be sent if temperature above this value (centiCelsius: 2550 is 25,50Â°)
  int temperatureLowThreshold;   // Alert will be sent if temperature below this value (centiCelsius: 2400 is 24Â°)
  lightSchedule lightOn;         // Interval during which lights should be on
  lightSchedule lightOff;        // Interval during which lights should be off
  int highLevelPinValue;         // Value (HIGH or LOW) on level pin when water level is ok
  phoneConfig registeredNumbers[MAX_PHONE_NUMBERS];  // Phone numbers to send alerts to, according to their subscriptions and permissions
  int powerThreshold;            // Alert will be sent if main power input pin below this value
} config;

#define ONE_STATUS_MSG_LENGTH 20
struct displayData {
  char statusMessage[300];
  // the message is scrolling across the first display line
  // offset keeps track of the first message char to display
  unsigned char offset = 0;
  // TODO size below should somehow be LCD width, but what if LCD disabled with compilation directives ?
  char alertMessage[17]; // Message displayed permanently
  // Transient message will be displayed for short durations
  unsigned long transientStartDisplayTime=0;
  unsigned long transientDisplayDuration=2000; // 2 seconds
  unsigned long refreshPeriod = 300;
  unsigned long lastRefresh = 0;
} display;

struct measureData {
  int light = 0;
  boolean lightAlert = false;
  int powerLevel = 0;
  boolean powerAlert = false;
  int temperature; // decadegrees: 2500 for 25.00°
  boolean temperatureAlert = false;
  boolean waterLevelAlert = false;
  boolean oneAlert = false;
} measures;

// Flags for services subscriptions and permissions
#define FLAG_SERVICE_ALERT   0x01
#define FLAG_SERVICE_EVENT   0x02     // TODO : not implemented yet

#define FLAG_ADMIN           0x80

// GSM
// initialize the library instance
#define PINNUMBER "" // Pin number for the SIM CARD
GSM gsmAccess; // include a 'true' parameter for debug enabled
GSM_SMS sms;
GSMVoiceCall vcs;

// LCD
#if WITH_LCD_SUPPORT
// TODO check if still true: #include are processed no matter what (known bug) : comment or uncomment them is the only way
#include <LiquidCrystal.h>
// initialize the library with the interface pins
// GMS uses 2 and 3 (at least)
LiquidCrystal lcd(4,5,6,8,9,13);
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#endif

#define LIGHT_PIN 0         // Analog input for light sensor
#define MAIN_POWER_PIN 1    // Analog input to monitor main power
#define TEMPERATURE_PIN 12

/* DS18B20 Temperature chip i/o */
OneWire  ds(TEMPERATURE_PIN);
#define MAX_DS1820_SENSORS 1
byte addr[MAX_DS1820_SENSORS][8];

// Okay these globals are pretty bad and may still be the cause of a few bugs, but the
// small amount of variable space on UNO left me with little choice.
// May be worth redesigning all this since using Mega, now...
#define PROGMEM_MSG_MAX_SIZE 60
char progMemMsg[PROGMEM_MSG_MAX_SIZE + 1];

#define TEMPERATURE_CHECK_PERIOD 5000
unsigned long lastTemperatureCheck = millis();

#define LIGHT_CHECK_PERIOD 5000
unsigned long lastLightCheck = lastTemperatureCheck;

#define SERIAL_STATUS_PERIOD 5000
unsigned long lastSerialStatus = lastTemperatureCheck;

#define LEVEL_CHECK_PERIOD 5000
unsigned long lastLevelCheck = lastTemperatureCheck;

#define POWER_CHECK_PERIOD 5000
unsigned long lastPowerCheck = lastTemperatureCheck;

// Check for incoming sms every 30 seconds
#define SMS_CHECK_PERIOD 30000
unsigned long lastSmsCheck = lastTemperatureCheck;

// Check for incoming voice call: if not handled, will break SMS management
#define CALL_CHECK_PERIOD 6000
unsigned long lastCallCheck = lastTemperatureCheck;

// Max size of a received SMS
#define MAX_SMS_LENGTH 40
boolean gsmEnabled = false;    // TSTWIFI

#define LEVEL_PIN 11

char serialMessage[MAX_SERIAL_INPUT_MESSAGE];
char serialMessageFromESP[MAX_SERIAL_INPUT_MESSAGE];

// TODO: offer choice with DS1302 ?
DS1307 clock; // The RTC handle to get date and time

// System initialization
void setup(void) {
  serialMessage[0] = 0;
  serialMessageFromESP[0] = 0;
  Serial.begin(9600);
  Serial1.begin(115200);

  // Init pin with the level detector as input
  pinMode(LEVEL_PIN, INPUT);
  digitalWrite(LEVEL_PIN, HIGH); // Activate pullup resistor

  // During dev and debug, in case some loop goes crazy, limit the number of EEPROM writes to save it
  // TODO : this should be removed some day
  EEPROM.setMaxAllowedWrites(100);

  initLCD();

  displayTransient(getProgMemMsg(INIT_AQUAMON_MSG));
  Serial.println(getProgMemMsg(BUILD_MSG));

  // Initialise RTC
  clock.begin();

  clock.getTime();
  if(clock.year == 0) {
    setupClock();
  }

  readConfig();
  displayConfig(false, "");

  displayTransient(getProgMemMsg(TEMP_INIT_MSG));
  if (!ds.search(addr[0])) {
    displayTransient(getProgMemMsg(ADDR_ERR_MSG));
    ds.reset_search();
    delay(250);
  }
  if(gsmEnabled) {
    delay(1000);
    // Start GSM shield
    displayTransient(getProgMemMsg(INIT_GSM_MSG));
    displayTransient(getProgMemMsg(CONNECTING_GSM_MSG));
    if(gsmAccess.begin(PINNUMBER)==GSM_READY) {
      displayTransient(getProgMemMsg(CONNECTED_GSM_MSG));
      // This makes sure the modem notifies correctly incoming events
      vcs.hangCall();
    } else {
      displayTransient(getProgMemMsg(NOT_CONNECTED_GSM_MSG));
      delay(1000);
    }
  }
#if WITH_LCD_SUPPORT
  lcd.clear();
#endif
  // wait a couple seconds before starting surveillance, to avoid dummy temperature
  delay(2000);
}

// Returns pointer to the global string buffer holding a string read from PROGMEM
// This is not good but I was trying to save as much memory as possible when trying
// to make the whole thing fit on Arduino Uno.
char * getProgMemMsg(int messageId) {
  strncpy_P(progMemMsg, (char*)pgm_read_word(&messages[messageId]), PROGMEM_MSG_MAX_SIZE);
  // Just in case
  progMemMsg[PROGMEM_MSG_MAX_SIZE] = 0;
  return progMemMsg;
}

void loop(void) {
  // RTC does not have an epoch field :(
  unsigned long now = millis();
  int incomingChar = 0;
  int length;

  // Sensors are not checked at each cycle either.
  if(checkElapsedDelay(now, lastLightCheck, LIGHT_CHECK_PERIOD)) {
    checkLight();
    lastLightCheck = now;
  }
  if(checkElapsedDelay(now, lastTemperatureCheck, TEMPERATURE_CHECK_PERIOD)) {
    checkTemperature();
    lastTemperatureCheck = now;
  }
  if(checkElapsedDelay(now, lastLevelCheck, LEVEL_CHECK_PERIOD)) {
    checkWaterLevel();
    lastLevelCheck = now;
  }
  if(checkElapsedDelay(now, lastPowerCheck, POWER_CHECK_PERIOD)) {
    checkPower();
    lastPowerCheck = now;
  }
  measures.oneAlert = measures.lightAlert || measures.powerAlert || measures.temperatureAlert || measures.waterLevelAlert;

  // Display is not refreshed at each cycle but only every refreshPeriod
  if(checkElapsedDelay(now, display.lastRefresh, display.refreshPeriod)) {
    refreshDisplay();
  }

  if(measures.oneAlert) {
    sendAlert();
  }

  if(checkElapsedDelay(now, lastSmsCheck, SMS_CHECK_PERIOD)) {
    checkSMS();
    lastSmsCheck = millis(); // checkSMS is a bit slow.
  }
  if(checkElapsedDelay(now, lastCallCheck, CALL_CHECK_PERIOD)) {
    checkCall();
    lastCallCheck = now;
  }

  // Check USB serial for incoming messages
  if(readFromSerial(&Serial, serialMessage, 500)) {
    processMessageFromSerial(serialMessage);
    serialMessage[0] = 0;
  }

  // Check serial1 for incoming messages from wifi module
  if(readFromSerial(&Serial1, serialMessageFromESP, 500)) {
    processMessageFromESP(serialMessageFromESP);
    serialMessageFromESP[0] = 0;
  }

}

// Process a message sent by the ESP module
// Messages starting with '@' are arduino commands, need to be processed like SMS received
// Messages starting with '#' are ESP requests
// Other messages just need to be forwarded to usb serial (debug and log stuff)
void processMessageFromESP(char *message) {
//Serial.println("got message");
//Serial.println(message);
  // Message not starting with '#' just needs to be forwarded to USB serial
  char *colonPosition;
  char *content;
  char *firstChar = message;
  char answer[2000];
  if(*firstChar == '@') {
    // The message sent by ESP is actually a command for the arduino like the one it gets from
    // the usb serial or sms
    firstChar++; // check after first # char
    Serial.println(firstChar);
    processMessageFromSerial(firstChar);
  } else if(*firstChar == '#') {
    // The message sent by ESP is a specific ESP request
    firstChar++; // check after first # char
    colonPosition = strchr(firstChar, ':');
    if(colonPosition != NULL) {
      content = colonPosition + 1;
    }
    if(strncmp(firstChar, REQUEST_IF_GSM, strlen(REQUEST_IF_GSM)) == 0) {
      // esp wants to know if module is equipped with GSM
      sprintf(answer, "%s:%d", REQUEST_IF_GSM, gsmEnabled);
      Serial1.println(answer);
    } else if(strncmp(firstChar, REQUEST_MEASURES, strlen(REQUEST_MEASURES)) == 0) {
      char date[40]; // No need for 40, unless there is no rtc connected :)
      getCurrentDate(date);
      Serial.println("ESP wants measures");
      // esp wants to know measures to log them
      // We'll pass a json object using ArduinoJson library by Benoît Blanchon
      // size : https://rawgit.com/bblanchon/ArduinoJson/master/scripts/buffer-size-calculator.html
      StaticJsonBuffer<400> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["temp"] = measures.temperature;
      root["tempAlert"] = measures.temperatureAlert;
      root["minTemp"] = config.temperatureLowThreshold;
      root["maxTemp"] = config.temperatureHighThreshold;
      root["light"] = measures.light;
      root["lightAlert"] = measures.lightAlert;
      root["minOnLight"] = config.lightOn.minAcceptableValue;
      root["maxOnLight"] = config.lightOn.maxAcceptableValue;
      root["minOffLight"] = config.lightOff.minAcceptableValue;
      root["maxOffLight"] = config.lightOff.maxAcceptableValue;
      root["waterLevelAlert"] = measures.waterLevelAlert;
      root["powerAlert"] = measures.powerAlert;
      root["oneAlert"] = measures.oneAlert;
      root["date"] = date;
      sprintf(answer, "%s:", REQUEST_MEASURES);
      firstChar = answer;
      firstChar += strlen(answer);
      root.printTo(firstChar, sizeof(answer) - strlen(answer));      // ok, not pretty...
      //Serial1.println(answer);
      //Serial.println(answer);
      //writeToSerial(&Serial, answer, 500);
      writeToSerial(&Serial1, answer, 500);
    }

  } else {
    // The message sent by ESP should just be sent to USB serial
    //Serial.println(message);
    writeToSerial(&Serial, message, 500);
  }
}

// Process a message received by USB serial
void processMessageFromSerial(char *message) {
  if(*message == '#') {
    // Simulates a message received from ESP: process it as such
    processMessageFromESP(message);
  } else {
    processMessage(message, "");
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

// Check water level on digital input. Returns false if low
void checkWaterLevel() {
  measures.waterLevelAlert = !(config.highLevelPinValue == digitalRead(LEVEL_PIN));
}

// return true if temperature is within low and high thresholds
void checkTemperature() {
  int highByte, lowByte, tReading, signBit, tc_100;
  byte sensor = 0;

  byte i;
  byte present = 0;
  byte data[12];

  if ( OneWire::crc8( addr[sensor], 7) != addr[sensor][7]) {
    displayTransient(getProgMemMsg(CRC_NOT_VALID_MSG));
  } else if ( addr[sensor][0] != 0x28) {
    // When testing with no sensor, set fake temperature at 25.25°
    measures.temperature = 2525;
    measures.temperatureAlert = false;
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
    measures.temperature = tc_100;
    if((tc_100 < config.temperatureLowThreshold) || (tc_100 > config.temperatureHighThreshold)) {
      measures.temperatureAlert = true;
    } else {
      measures.temperatureAlert = false;
    }
  }
}

// Return true if main power is ok
boolean checkPower() {
  measures.powerLevel = analogRead(MAIN_POWER_PIN);
  // If power level less than threshold => not ok
  if(measures.powerLevel < config.powerThreshold) {
    measures.powerAlert = true;
  } else {
    measures.powerAlert = false;
  }
}

// Check if light level is within acceptable limits depending on current hour
boolean checkLightSchedule(int lightLevel, lightSchedule schedule) {
  boolean result = true;
  unsigned int startMin;
  unsigned int endMin;
  unsigned int nowMin;
  unsigned int midnight = 23*60+59;

  clock.getTime();
  startMin = schedule.startHour * 60 + schedule.startMinute;
  endMin = schedule.endHour * 60 + schedule.endMinute;
  nowMin = clock.hour * 60 + clock.minute;
  // if interval starts before midnight and ends after midnight (generally 'lights off' interval)
  // then adjustements are needed
  if(startMin > endMin) {
    if(nowMin <= startMin) {
      startMin = 0;
    } else {
      endMin = midnight;
    }
  }
  // Is current time within schedule start and end ?
  if((nowMin >= startMin) && (nowMin <= endMin)) {
    if(lightLevel < schedule.minAcceptableValue || lightLevel > schedule.maxAcceptableValue) {
      result = false;
    }
  }
  return(result);
}

// Return true if light level is within boundaries depending on current time
void checkLight() {
  measures.light = analogRead(LIGHT_PIN);
  measures.lightAlert = !checkLightSchedule(measures.light, config.lightOn) || !checkLightSchedule(measures.light, config.lightOff);
}

// Check for incoming voice call, in order to not break SMS
void checkCall() {
  switch (vcs.getvoiceCallStatus()) {
    case RECEIVINGCALL:
      vcs.answerCall();
      delay(1000);
      vcs.hangCall();
      break;
  }
}

// Check for incoming SMS, and processes it if any
void checkSMS() {
  char from[PHONE_NUMBER_LENGTH + 1];
  char msgIn[MAX_SMS_LENGTH + 1];
  char c,i;
  int cptr = 0;
  if(!gsmEnabled) {
    Serial.println("NO GSM");
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
      processMessage(msgIn, from);
    }
  }
}

void processMessage(char *msgIn, char *from) {
  Serial.println(msgIn);
  // Messages starting with 'wifi:' are forwarded to wifi module
  if(strncmp(msgIn, "wifi:", 5) == 0) {
    Serial1.println(&(msgIn[5]));
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_INTERVAL))) {   // Sender wants to set his alert minimum interval (seconds)
    setAlertInterval(from, msgIn);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_TEMP_ADJ))) {  // Sender wants to set temperature adjustment
    setTemperatureAdjustment(from, msgIn);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SUBS))) {   // Sender wants to receive subscription information
    sendSubs(from);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_CONFIG))) {   // Sender wants to receive configuration
    displayConfig(true, from);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SAVE))) {    // Sender wants config to be saved to EEPROM
    saveConfig(from);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_RESET_CONFIG))) {   // Sender wants to reset configuration to previously saved values
    resetConfig(from);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_ABOUT))) {   // Sender wants to receive 'about' information
    sendAbout(from);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_TEMP))) {   // Sender wants to set temperture thresholds
    setTemperatureThresholds(from, msgIn);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_LIGHT_THRESHOLD))) {  // Sender wants to set light threshold
    setLightThresholds(from, msgIn);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_LIGHT_SCHEDULE))) {  // Sender wants to set light schedule
    setLightSchedule(from, msgIn);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_STATUS))) {  // Sender wants to receive current measurements
    sendStatus(from);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SUB))) {    // Sender wants to subscribe to given service
    subscribe(from, msgIn);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_CLEAR_ALERT))) {    // Sender wants to clear his alert delay
    clear(from);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_UNSUB))) {   // Sender wants to unsubscribe to give service
    unsubscribe(from, msgIn);
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_RESET_SUB))) {  // Admin Sender wants to cancel all subscriptions
    if(!checkAdmin(from)) {
      sendSMS(from, getProgMemMsg(ACCESS_DENIED_MSG));
    } else {
      resetSub();
      sendSMS(from, getProgMemMsg(RESET_SUB_DONE_MSG));
    }
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_RESET_LCD))) {  // Sender wants to reset the display
    initLCD();
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SET_ADMIN))) {  // Admin Sender wants to set another admin phone number
    if(!checkAdmin(from)) {
      sendSMS(from, getProgMemMsg(ACCESS_DENIED_MSG));
    } else {
      setAdmin(from, msgIn);
    }
  } else if(msgIn == strstr(msgIn, getProgMemMsg(IN_SMS_SET_TIME))) {  // Sender wants to set date and time 
    setTime(from, msgIn);
  } else {
    displayTransient(getProgMemMsg(UNKNOWN_MSG));
    sendSMS(from, getProgMemMsg(UNKNOWN_MSG));
  }
}

// Return the flag for a given service name
byte getServiceFlagFromName(char *serviceName) {
  byte serviceFlag = 0;

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
void setLightThresholds(char *from, char *msgIn) {
  int min, max;
  char onOff[5];
  boolean valid = false;
  sscanf(msgIn, getProgMemMsg(IN_SMS_LIGHT_THRESHOLD_FORMAT), onOff, &min, &max);
  onOff[4] = 0; // just in case. Probably too late anyway :)
  if(strncmp(onOff, "on", 2) == 0) {
    config.lightOn.minAcceptableValue = min;
    config.lightOn.maxAcceptableValue = max;
    valid = true;
  }
  if(strncmp(onOff, "off", 3) == 0) {
    config.lightOff.minAcceptableValue = min;
    config.lightOff.maxAcceptableValue = max;
    valid = true;
  }
  if(valid) {
    sendSMS(from, getProgMemMsg(LIGHT_THRESHOLD_SET_MSG));
  } else {
    sendSMS(from, getProgMemMsg(UNKNOWN_MSG));
  }
}

// Set the light threshold below which an alert will be sent
void setLightSchedule(char *from, char *msgIn) {
  unsigned int startHour, startMinute, endHour, endMinute;
  lightSchedule *schedule; 
  char onOff[5];
  boolean valid = false;
  sscanf(msgIn, getProgMemMsg(IN_SMS_LIGHT_SCHEDULE_FORMAT), onOff, &startHour, &startMinute, &endHour, &endMinute);
  onOff[4] = 0; // just in case. Probably too late anyway :)
  if(strncmp(onOff, "on", 2) == 0) {
    schedule = &config.lightOn;
    valid = true;
  }
  if(strncmp(onOff, "off", 3) == 0) {
    schedule = &config.lightOff;
    valid = true;
  }
  if(valid) {
    schedule->startHour = (byte)startHour;
    schedule->startMinute = (byte)startMinute;
    schedule->endHour = (byte)endHour;
    schedule->endMinute = (byte)endMinute;
    sendSMS(from, getProgMemMsg(LIGHT_SCHEDULE_SET_MSG));
  } else {
    sendSMS(from, getProgMemMsg(UNKNOWN_MSG));
  }
}

// Admin can change the admin number
void setAdmin(char *from, char *msgIn) {
  char newAdmin[PHONE_NUMBER_LENGTH + 1];
  sscanf(msgIn, getProgMemMsg(IN_SMS_SET_ADMIN_FORMAT), newAdmin); // not really safe
  newAdmin[PHONE_NUMBER_LENGTH] = 0;  // Make sure...
  subscribeFlag(newAdmin, FLAG_ADMIN | FLAG_SERVICE_ALERT);
  sendSMS(from, getProgMemMsg(SET_ADMIN_DONE_MSG));
}

// Set date 
void setTime(char *from, char *msgIn) {
  // if incoming message is more than just "time", it's a set time message with arguments
    Serial.println(msgIn);
  if(strlen(msgIn) > strlen(getProgMemMsg(IN_SMS_SET_TIME) + 5)) {
    int day, month, year, hour, minute;
    sscanf(msgIn, getProgMemMsg(IN_SMS_SET_TIME_FORMAT), &year, &month, &day, &hour, &minute); // not really safe
    clock.fillByYMD(year, month, day);
    clock.fillByHMS(hour, minute, 30 );  // won't be very accurate because of SMS delays and reading period 
    clock.setTime();
    sendSMS(from, getProgMemMsg(SET_TIME_DONE_MSG));
  } else {
    sendSMS(from, getProgMemMsg(UNKNOWN_MSG));
  }

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
  byte serviceFlag;
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

boolean subscribeFlag(char *number, byte flag) {
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
      config.registeredNumbers[foundOrFree].lastAlertSmsTime = 0UL;
      config.registeredNumbers[foundOrFree].minAlertInterval = DEFAULT_MIN_ALERT_INTERVAL;
      done = true;
    }
  }
  return done;
}

// Clear the last time an alert was sent, so that new alerts can be sent before the interval is elapsed
void clear(char *number) {
  boolean found = false;
  char foundOrFree;
  found = findRegisteredNumber(number, &foundOrFree);
  if(found) {
    config.registeredNumbers[foundOrFree].lastAlertSmsTime = 0UL;
    sendSMS(number, getProgMemMsg(CLEAR_ALERT_DONE_MSG));
  }
}

// Unsubscribe a user from a service
void unsubscribe(char *number, char *msgIn) {
  byte serviceFlag;
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

  for(i=0; i < MAX_PHONE_NUMBERS; i++) {
    // If phone number initialized AND subscribed to the alert service
    if((config.registeredNumbers[i].number[0] != 0) && (0 != (config.registeredNumbers[i].permissionFlags & FLAG_SERVICE_ALERT)) ) {
      // If enough time since last alert SMS was sent to this number, send a new one
      if(checkElapsedDelay(now, config.registeredNumbers[i].lastAlertSmsTime, config.registeredNumbers[i].minAlertInterval)) {
        sendSMS(config.registeredNumbers[i].number, display.statusMessage);
        config.registeredNumbers[i].lastAlertSmsTime = now;
      }
    }
  }
}

// Send an SMS with the status to the given phone number
void sendStatus(char *toNumber) {
  sendSMS(toNumber, display.statusMessage);
}

// Send any kind of SMS to any give number
void sendSMS(char *toNumber, char *message) {
  char msg[200];
  // If message comes from progmem, it's in a global :(
  // that will be re used a few lines below when calling again getProgMemMsg
  strncpy(msg, message, 199);
  msg[199] = 0;
  Serial.println(msg);
  if(toNumber[0] == 0) return;

  // send the message
  if(gsmEnabled) {
    displayTransient(getProgMemMsg(SENDING_SMS_MSG));
    sms.beginSMS(toNumber);
    sms.print(msg);
    sms.endSMS();
    displayTransient(getProgMemMsg(SMS_SENT_MSG));
  }
}

void sendAbout(char *toNumber) {
  sendSMS(toNumber, getProgMemMsg(BUILD_MSG));
}

// Date as string but not localized (for ESP)
void getCurrentDate(char *message) {
  clock.getTime();
  sprintf(message, "%4d/%2d/%2d %2d:%2d",
     clock.year+2000, clock.month, clock.dayOfMonth,
     clock.hour, clock.minute);

}

// Log config to Serial console, and optionally send by sms
void displayConfig(boolean sendSMS, char *toNumber) {
  unsigned char i;
  char message[160];
  char strFlags[5];
  char space[] = ", ";
  if(!gsmEnabled || (toNumber[0] == 0)) {
    sendSMS = false;
  }

  sprintf(message, getProgMemMsg(BUILD_MSG));
  Serial.println(message);
  if(sendSMS) {
    sms.beginSMS(toNumber);
    sms.print(message);
    sms.print(space);
  }
  clock.getTime();
  sprintf(message, getProgMemMsg(CURRENT_DATE_FORMAT_MSG),
     clock.year+2000, clock.month, clock.dayOfMonth,
     clock.hour, clock.minute);

  Serial.println(message);
  if(sendSMS) {
    sms.print(message);
    sms.print(space);
  }

  sprintf(message, getProgMemMsg(TEMPERATURE_THRESHOLD_MSG_FORMAT), config.temperatureLowThreshold, config.temperatureHighThreshold);
  Serial.println(message);
  if(sendSMS) {
    sms.print(message);
    sms.print(space);
  }
  sprintf(message, getProgMemMsg(TEMPERATURE_ADJUSTMENT_MSG_FORMAT), config.temperatureAdjustment);
  Serial.println(message);
  if(sendSMS) {
    sms.print(message);
    sms.print(space);
    // Message needs to be less than 160c, all won't fit
    sms.endSMS();
    displayTransient(getProgMemMsg(SENDING_SMS_MSG));
  }

  sprintf(message, getProgMemMsg(LIGHT_SCHEDULE_MSG_FORMAT), "on",
                config.lightOn.startHour, config.lightOn.startMinute,
                config.lightOn.endHour, config.lightOn.endMinute);
  Serial.println(message);
  if(sendSMS) {
    // new sms for light config
    sms.beginSMS(toNumber);
    sms.print(message);
    sms.print(space);
  }
  sprintf(message, getProgMemMsg(LIGHT_THRESHOLD_MSG_FORMAT), "on", config.lightOn.minAcceptableValue, config.lightOn.maxAcceptableValue);
  Serial.println(message);
  if(sendSMS) {
    sms.print(message);
    sms.print(space);
  }

  sprintf(message, getProgMemMsg(LIGHT_SCHEDULE_MSG_FORMAT), "off",
                config.lightOff.startHour, config.lightOff.startMinute,
                config.lightOff.endHour, config.lightOff.endMinute);
  Serial.println(message);
  if(sendSMS) {
    sms.print(message);
    sms.print(space);
  }
  sprintf(message, getProgMemMsg(LIGHT_THRESHOLD_MSG_FORMAT), "off", config.lightOff.minAcceptableValue, config.lightOff.maxAcceptableValue);
  Serial.println(message);
  if(sendSMS) {
    sms.print(message);
    sms.print(space);
    sms.endSMS();
    displayTransient(getProgMemMsg(SENDING_SMS_MSG));
  }

  // This part not sent in config SMS (available in reply to 'subs' sms)
  for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
    if(config.registeredNumbers[i].number[0] != 0) {
      Serial.print(" ");
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

// Send a message listing all subscriptions
void sendSubs(char *toNumber) {
  char i;
  char message[40];
  unsigned long now = millis();
  if(!checkAdmin(toNumber)) {
    sendSMS(toNumber, getProgMemMsg(ACCESS_DENIED_MSG));
    return;
  }

  if(gsmEnabled) {
    sms.beginSMS(toNumber);
    for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
      if(config.registeredNumbers[i].number[0] != 0) {
        // I can't explain why but if I try to add a %d in the format below, the value is always 0
        sprintf(message, "%s %02X %ds ", config.registeredNumbers[i].number,
         config.registeredNumbers[i].permissionFlags, 
         config.registeredNumbers[i].minAlertInterval / 1000);
        sms.print(message);
        // I had to split the message to make it work... :(
        sprintf(message, "%ds ,", min(config.registeredNumbers[i].lastAlertSmsTime, (now - config.registeredNumbers[i].lastAlertSmsTime) / 1000));
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
    config.temperatureLowThreshold = 2400;

    // default setting for light on schedule
    config.lightOn.startHour = 13;           // Hour of the begining of light on
    config.lightOn.startMinute = 30;         // Minute of the begining of light on
    config.lightOn.endHour = 20;             // Hour of the ending of light on
    config.lightOn.endMinute = 30;           // Minute of the ending of light on
    config.lightOn.minAcceptableValue = 800;   // Min value for light sensor considered ok for light on
    config.lightOn.maxAcceptableValue = 1024;  // Max value for light sensor considered ok for light on

    // default setting for light off
    config.lightOff.startHour = 21;           // Hour of the begining of light off
    config.lightOff.startMinute = 00;         // Minute of the begining of light off
    config.lightOff.endHour = 13;             // Hour of the ending of light off
    config.lightOff.endMinute = 00;           // Minute of the ending of light off
    config.lightOff.minAcceptableValue = 0;   // Min value for light sensor considered ok for light off
    config.lightOff.maxAcceptableValue = 400; // Max value for light sensor considered ok for light off

    config.highLevelPinValue = HIGH;
    config.powerThreshold = 50;    // Power is between 0 and 1023

    // Reset all subscriptions
    resetSub();
    // Save new config to eeprom
    saveConfig("");
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
    config.registeredNumbers[i].minAlertInterval = DEFAULT_MIN_ALERT_INTERVAL; // Every 30 minutes = 1800 seconds
    config.registeredNumbers[i].lastAlertSmsTime = 0UL;
  }
}

// Save the configuration to EEPROM
void saveConfig(char *toNumber) {
  displayTransient(getProgMemMsg(SAVING_CONFIG_MSG));
  EEPROM.writeBlock(CONFIG_ADDRESS, config);
  sendSMS(toNumber, getProgMemMsg(CONFIG_SAVED_MSG));
}

// Rollback config to previously saved values (side effect: clears last time alert sms was sent)
void resetConfig(char *toNumber) {
  EEPROM.readBlock(CONFIG_ADDRESS, config);
  sendSMS(toNumber, getProgMemMsg(CONFIG_RESET_MSG));
}

// In charge of displaying messages.
// The scrolled message provides all measures, scrolling across the first display line
void refreshDisplay() {
  char scrolledMessage[200];
  char transferChar;
  int remaining;
  unsigned long now = millis();
  char waterLevelMessage[50];
  char powerMessage[50];
  char format[50];
  char temperature[6] ;

  // AVR does not support %f format in *printf functions. Too bad.
  sprintf(temperature, "%04d", measures.temperature);
  temperature[5] = 0;
  temperature[4] = temperature[3];
  temperature[3] = temperature[2];
  temperature[2] = '.';
  display.alertMessage[0] = 0;
  if(measures.lightAlert) {
    strcpy(display.alertMessage, getProgMemMsg(LIGHT_ALERT_MSG));
  }
  if(measures.waterLevelAlert) {
    strcpy(display.alertMessage, getProgMemMsg(LEVEL_ALERT_MSG));
    strcpy(waterLevelMessage, getProgMemMsg(LEVEL_LOW_MSG));
  } else {
    strcpy(waterLevelMessage, getProgMemMsg(LEVEL_HIGH_MSG));
  }
  if(measures.temperatureAlert) {
    strcpy(display.alertMessage, getProgMemMsg(TEMPERATURE_ALERT_MSG));
  }
  if(measures.powerAlert) {
    strcpy(display.alertMessage, getProgMemMsg(POWER_ALERT_MSG));
    strcpy(powerMessage, getProgMemMsg(POWER_OFF_MSG));
  } else {
    strcpy(powerMessage, getProgMemMsg(POWER_ON_MSG));
  }
  display.alertMessage[16] = 0;

  if(measures.oneAlert) {
    sprintf(display.statusMessage, getProgMemMsg(MEASURE_ALERT_MSG_FORMAT),
                   display.alertMessage,
                   temperature,
                   waterLevelMessage,
                   measures.light,
                   powerMessage);
  } else {
    sprintf(display.statusMessage, getProgMemMsg(MEASURE_MSG_FORMAT),
                   temperature,
                   waterLevelMessage,
                   measures.light,
                   powerMessage);
  }

  display.lastRefresh = millis();
  // Do not send to serial as often !
  if(checkElapsedDelay(now, lastSerialStatus, SERIAL_STATUS_PERIOD)) {
    Serial.println(display.statusMessage);
    lastSerialStatus = now;
  }

  #if WITH_LCD_SUPPORT
  // Handle scrolling message on display first line
  strcat(display.statusMessage, ", "); // scrolling message has no begining, no end
  remaining = strlen(display.statusMessage) - display.offset;
  strncpy(scrolledMessage, display.statusMessage + display.offset, 16);
  strncat(scrolledMessage, display.statusMessage, display.offset );
  scrolledMessage[16] = 0;
  display.offset++;
  if(remaining == 0) {
    display.offset = 0;
  }
  lcd.setCursor(0,0);
  lcd.print(scrolledMessage);

  if(checkElapsedDelay(millis(), display.transientStartDisplayTime, display.transientDisplayDuration)) {
    lcd.clear();
    displayAlertMessage();
  }

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

void displayAlertMessage() {
  if(display.alertMessage[0] != 0) {
    while(strlen(display.alertMessage) < 16) {
      strcat(display.alertMessage, " ");
    }
    #if WITH_LCD_SUPPORT
    lcd.setCursor(0,1);
    lcd.print(display.alertMessage);
    #endif
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


