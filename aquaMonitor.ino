#include <OneWire.h>
#include <LiquidCrystal.h>
#include <GSM.h>
#include <IRremote.h>
#include <EEPROMex.h>
#include <EEPROMvar.h>

#define PHONE_NUMBER_LENGTH 15
#define MAX_PHONE_NUMBERS 4
#define CONFIG_ADDRESS 0
// Change this version to reset the EEPROM saved configuration
#define CONFIG_VERSION 2

struct phoneConfig {
  unsigned char serviceFlags;
  char number[PHONE_NUMBER_LENGTH + 1];
};

struct eepromConfig {
  unsigned char version;
  int temperatureAdjustment;
  int temperatureHighThreshold;
  int temperatureLowThreshold;
  int lightThreshold;
  phoneConfig registeredNumbers[MAX_PHONE_NUMBERS];
} config;

#define FLAG_SERVICE_ALERT   0x01
#define FLAG_SERVICE_EVENT   0x02
#define FLAG_SERVICE_SUB     0x04
#define FLAG_SERVICE_UNSUB   0x08

// Init IR
#define IR_PIN 5
IRrecv reception_ir(IR_PIN);
decode_results code;
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

// GSM
// initialize the library instance
#define PINNUMBER "" // Pin number for the SIM CARD
GSM gsmAccess; // include a 'true' parameter for debug enabled
GSM_SMS sms;

// defined in a private file haha !
#include <remoteNumber.h>

// LCD
// initialize the library with the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11 , 12);

#define LCD_WIDTH 16
#define LCD_HEIGHT 2
#define MSG_MAX_LENGTH 14

#define LIGHT_PIN 0
#define TEMPERATURE_PIN 6

/* DS18B20 Temperature chip i/o */
OneWire  ds(TEMPERATURE_PIN);
#define MAX_DS1820_SENSORS 1
byte addr[MAX_DS1820_SENSORS][8];

// Okay these globals are pretty bad and still the cause of a few bugs, but the
// small amount of variable space left me with little choice.
char msgBuf[70];
char progMemMsg[60];
char temperatureMsg[20];
char lightMsg[20];

#define TEMPERATURE_CHECK_PERIOD 5000
unsigned long lastTemperatureCheck = millis() - TEMPERATURE_CHECK_PERIOD;

#define LIGHT_CHECK_PERIOD 5000
unsigned long lastLightCheck = millis() - LIGHT_CHECK_PERIOD;

#define IR_DISPLAY_TIMEOUT 250
unsigned long lastIrDisplay = 0;

// Check for incoming sms every 30 seconds
#define SMS_CHECK_PERIOD 30000
unsigned long lastSmsCheck = 0;

#define MIN_SMS_SEND_DELAY 30000
unsigned long lastSmsSent = 0;
// Max size of a received SMS
#define MAX_SMS_LENGTH 20
boolean gsmEnabled = !false;

boolean statusOK = true;
#include <avr/pgmspace.h>

// Messages and formats saved in  to save RAM
const char initAquamonMsg[] PROGMEM = {"Init AquaMon"};
const char initIRMsg[] PROGMEM = {"Init IR remote"};
const char tempInitMsg[] PROGMEM = {"DS1820 Test"};
const char addrErrMsg[] PROGMEM = {"Error addr 0"};
const char initGSMMsg[] PROGMEM = {"GSM init."};
const char connectingGSMMsg[] PROGMEM = {"GSM Connecting"};
const char connectedGSMMsg[] PROGMEM = {"GSM Connected"};
const char notConnectedGSMMsg[] PROGMEM = {"Not connected"};
const char crcNotValidMsg[] PROGMEM = {"CRC is not valid"};
const char familyMsg[] PROGMEM = {"Not DS18B20 family."};
const char temperatureMsgFormat[] PROGMEM = {"Temp: %c%d.%d"};
const char lightMsgFormat[] PROGMEM = {"Light: %d"};
const char checkSMSMsg[] PROGMEM = {"Checking SMS"};
const char fromNumberMsg[] PROGMEM = {"Msg received from:"};
const char discardSMSMsg[] PROGMEM = {"Discard SMS"};
const char sendingSMSMsg[] PROGMEM = {"Sending SMS..."};
const char lightThresholdMsgFormat[] PROGMEM = {"Light th: %d"};
const char temperatureAdjustmentMsgFormat[] PROGMEM = {"Temp Adj: %d"};
const char temperatureThresholdMsgFormat[] PROGMEM = {"Temp: %d %d"};
const char numberSubscribedMsg[] PROGMEM = {"Number %s was subscribed to service %s"};
const char numberNotSubscribedMsg[] PROGMEM = {"Number %s was NOT subscribed to service %s: full"};
const char numberUnsubscribedMsg[] PROGMEM = {"Number %s was unsubscribed to service %s"};
const char numberNotUnsubscribedMsg[] PROGMEM = {"Number %s was not unsubscribed to service %s: not found"};

#define INIT_AQUAMON_MSG 0
#define INIT_IR_MSG 1
#define TEMP_INIT_MSG 2
#define ADDR_ERR_MSG 3
#define INIT_GSM_MSG 4
#define CONNECTING_GSM_MSG 5
#define CONNECTED_GSM_MSG 6
#define NOT_CONNECTED_GSM_MSG 7
#define CRC_NOT_VALID_MSG 8
#define FAMILY_MSG 9
#define TEMPERATURE_MSG_FORMAT 10
#define LIGHT_MSG_FORMAT 11
#define CHECK_SMS_MSG 12
#define FROM_NUMBER_MSG 13
#define DISCARD_SMS_MSG 14
#define SENDING_SMS_MSG 15
#define LIGHT_THRESHOLD_MSG_FORMAT 16
#define TEMPERATURE_ADJUSTMENT_MSG_FORMAT 17
#define TEMPERATURE_THRESHOLD_MSG_FORMAT 18
#define NUMBER_SUBSCRIBED_MSG 19
#define NUMBER_NOT_SUBSCRIBED_MSG 20
#define NUMBER_UNSUBSCRIBED_MSG 21
#define NUMBER_NOT_UNSUBSCRIBED_MSG 22

const char* const messages[] PROGMEM = {initAquamonMsg, initIRMsg, tempInitMsg, addrErrMsg, initGSMMsg, connectingGSMMsg,
   connectedGSMMsg, notConnectedGSMMsg, crcNotValidMsg, familyMsg, temperatureMsgFormat, lightMsgFormat, checkSMSMsg,
   fromNumberMsg, discardSMSMsg, sendingSMSMsg, lightThresholdMsgFormat, temperatureAdjustmentMsgFormat,
   temperatureThresholdMsgFormat, numberSubscribedMsg, numberNotSubscribedMsg, numberUnsubscribedMsg, numberNotUnsubscribedMsg
};

void setup(void) {
  Serial.begin(9600);
  EEPROM.setMaxAllowedWrites(10);
  lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);

  print(0, 0, getProgMemMsg(INIT_AQUAMON_MSG));
  readConfig();
  //print(0,0, config.registeredNumbers[0].number);
  print(0, 1, getProgMemMsg(INIT_IR_MSG));
  reception_ir.enableIRIn(); // init receiver
  delay(250);

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
  lcd.clear();
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
  lcd.clear();
}

char * getProgMemMsg(int messageId) {
  strcpy_P(progMemMsg, (char*)pgm_read_word(&messages[messageId]));
  return progMemMsg;
}

void loop(void) {
  unsigned long now = millis();
  
  // Check if an IR code was received
  if (reception_ir.decode(&code)) { 
    processIRCode(code);
    reception_ir.resume(); // be ready for next code 
  }

  if((now - lastLightCheck) >= LIGHT_CHECK_PERIOD) {
    statusOK = statusOK && checkLight();
    lastLightCheck = now;
  }
  if((now - lastTemperatureCheck) >= TEMPERATURE_CHECK_PERIOD) {
    statusOK = statusOK && checkTemperature();
    lastTemperatureCheck = millis();
  }
  if((now - lastIrDisplay) >= IR_DISPLAY_TIMEOUT) {
    resetIRDisplay();
  }
  if((now - lastSmsCheck) >= SMS_CHECK_PERIOD) {
    checkSMS();
    lastSmsCheck = now;
  }

  if(!statusOK && (now - lastSmsSent) >= MIN_SMS_SEND_DELAY) {
    sendAlert();
    statusOK = true;
    lastSmsSent = now;
  }
}

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

// Check the light level, compares it to the config-defined light threshold
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
  //return true;
}

// Check for incoming SMS, and processes it if any
void checkSMS() {
  char from[PHONE_NUMBER_LENGTH + 1];
  char service[20];
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

    // An example of message disposal
    // Any messages starting with # should be discarded
    if (sms.peek() == '#') {
      Serial.println(getProgMemMsg(DISCARD_SMS_MSG));
      sms.flush();
    } else {
      // Read message bytes and print them
      while ((c = sms.read()) && (cptr < MAX_SMS_LENGTH)) {
        if ((c > 64) && (c < 91)) {
          c = c + 32;
        }
        msgBuf[cptr++] = c;
      }
      msgBuf[cptr] = 0;
      Serial.println(msgBuf);
      if(strncmp(msgBuf, "status", 6) == 0) {
        sendStatus(from);
      } else if(strncmp(msgBuf, "sub ", 3) == 0) {
        sscanf(msgBuf, "sub %s", service);
        subscribe(from, service);
      } else if(strncmp(msgBuf, "unsub ",5) == 0) {
        sscanf(msgBuf, "unsub %s", service);
        unsubscribe(from, service);
      } else if(strncmp(msgBuf, "reset sub", 9) == 0) {
        config.registeredNumbers[0].serviceFlags = 0xFF;
        strcpy(config.registeredNumbers[0].number, REMOTE_NUMBER);
        for(i=1 ; i < MAX_PHONE_NUMBERS; i++ ) {
          config.registeredNumbers[i].serviceFlags = 0;
          config.registeredNumbers[i].number[0] = 0;
        }
      }
      // Delete message from modem memory
      sms.flush();
    }
  }
}

unsigned char getServiceFlagFromName(char *serviceName) {
  unsigned char serviceFlag = 0;

  if(strncmp(serviceName, "alert", 5)) {
    serviceFlag = FLAG_SERVICE_ALERT;
  } else if(strncmp(serviceName, "event", 5)) {
    serviceFlag = FLAG_SERVICE_EVENT;
  } else if(strncmp(serviceName, "sub", 3)) {
    serviceFlag = FLAG_SERVICE_SUB;
  } else if(strncmp(serviceName, "unsub", 5)) {
    serviceFlag = FLAG_SERVICE_UNSUB;
  }
  Serial.println(serviceFlag);
  return serviceFlag;
}

void subscribe(char *number, char *serviceName) {
  unsigned char serviceFlag;
  unsigned char i, firstFree = MAX_PHONE_NUMBERS;
  boolean done = false;

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
      config.registeredNumbers[i].serviceFlags |= serviceFlag;
      done = true;
    }
    // If number was not yet registered but a free spot is available, register it
    if(!done && (firstFree < MAX_PHONE_NUMBERS)) {
      strncpy(config.registeredNumbers[firstFree].number, number, PHONE_NUMBER_LENGTH);
      config.registeredNumbers[firstFree].serviceFlags = serviceFlag;
      done = true;
    }
  }
  if(done) {
    sprintf(msgBuf, getProgMemMsg(NUMBER_SUBSCRIBED_MSG), number, serviceName);
    sendSMS(number, msgBuf);
  } else {
    sprintf(msgBuf, getProgMemMsg(NUMBER_NOT_SUBSCRIBED_MSG), number, serviceName);
    sendSMS(number, msgBuf);
  }
}

void unsubscribe(char *number, char *serviceName) {
  unsigned char serviceFlag;
  unsigned char i;
  boolean done = false;

  serviceFlag = getServiceFlagFromName(serviceName);
  // look for number
  for(i=0 ; i<MAX_PHONE_NUMBERS; i++) {
    if (0 == strncmp(config.registeredNumbers[i].number, number, PHONE_NUMBER_LENGTH)) {
      // If number found, cancel it
      config.registeredNumbers[i].serviceFlags &= !serviceFlag;
      if(0 == config.registeredNumbers[i].serviceFlags) {
        config.registeredNumbers[i].number[0] = 0;
      }
      done = true;
    }
  }
  if(done) {
    sprintf(msgBuf, getProgMemMsg(NUMBER_UNSUBSCRIBED_MSG), number, serviceName);
    sendSMS(number, msgBuf);
  } else {
    sprintf(msgBuf, getProgMemMsg(NUMBER_NOT_UNSUBSCRIBED_MSG), number, serviceName);
    sendSMS(number, msgBuf);
  }
}

// Send SMS to all numbers subscribed to 'alerts'
void sendAlert() {
  unsigned char i, flag;
  print(0, 1, "Alerts");
  for(i=0; i < MAX_PHONE_NUMBERS; i++) {
    // If phone number initialized AND subscribed to the alert service
    if((config.registeredNumbers[i].number[0] != 0) && (0 != (config.registeredNumbers[i].serviceFlags & FLAG_SERVICE_ALERT)) ) {
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

void sendSMS(char *toNumber, char *message) {
  print(0, 0, getProgMemMsg(SENDING_SMS_MSG));
  print(0, 1, "To: ");
  print(4, 1, toNumber);
  Serial.println(message);

  // send the message
  if(gsmEnabled) {
    sms.beginSMS(toNumber);
    sms.print(message);
    sms.endSMS();
    print(0, 0, "SMS sent");
  }
}

// Display a message on the LCD display
void print(int col, int row, const char* displayMsg) {
  char lcdBuf[LCD_WIDTH + 1];
  int cptr = 0;
  int count = strlen(displayMsg);
  Serial.println(displayMsg);

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
}

// Remove the IR indicators from the LCD display
void resetIRDisplay() {
  //lcd.setCursor(14,0);
  //lcd.print("  ");
  lcd.setCursor(15,1);
  lcd.print(" ");
}

// Check if an IR code was received and process it according to its value
void processIRCode(decode_results code) {
  unsigned long irCode = code.value;
  if((irCode == IR_REPEAT_CODE) && (0 != previousCode)) {
    irCode = previousCode;
  }
  previousCode = irCode;
  lastIrDisplay = millis();
  Serial.println(irCode, HEX);
  //print(14, 0, "ir");
  switch(irCode) {
    case IR_DISPLAY_PHONE_NUMBER:
      // loop over config
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
      saveConfig();
    break;
    case IR_LOG_CONFIG:
      logConfig();
    break;
    default:
      print(15, 1, "?");
  }
}

void logConfig() {
  char i;
  for(i=0 ; i < MAX_PHONE_NUMBERS; i++ ) {
    Serial.println(i);
    Serial.println(config.registeredNumbers[i].number);
    Serial.println(config.registeredNumbers[i].serviceFlags);
  }

}

void readConfig() {
  unsigned char i;
  print(0, 1, "Read Config");
  EEPROM.readBlock(CONFIG_ADDRESS, config);
  if(config.version != CONFIG_VERSION) {
    config.version = CONFIG_VERSION;
    config.lightThreshold = 500;
    config.temperatureAdjustment = 0;
    config.temperatureHighThreshold = 2700;
    config.temperatureLowThreshold = 2100;
    // Reset all saved numbers
    // First one is main number, hardcoded, and subscribed to all services
    config.registeredNumbers[0].serviceFlags = 0xFF;
    strcpy(config.registeredNumbers[0].number, REMOTE_NUMBER);
    for(i=1 ; i < MAX_PHONE_NUMBERS; i++ ) {
      config.registeredNumbers[i].serviceFlags = 0;
      config.registeredNumbers[i].number[0] = 0;
    }

    Serial.println("Saving");
    EEPROM.writeBlock(CONFIG_ADDRESS, config);
  }
}

void saveConfig() {
  print(0, 0, "Saving config");
  EEPROM.writeBlock(CONFIG_ADDRESS, config);
  print(0, 1, "Config Saved");
}
