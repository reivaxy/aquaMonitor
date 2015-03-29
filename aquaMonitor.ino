#include <OneWire.h>
#include <LiquidCrystal.h>
#include <GSM.h>
#include <IRremote.h>

#include <EEPROMex.h>
#include <EEPROMvar.h>

#define PHONE_NUMBER_LENGTH 15
#define MAX_PHONE_NUMBERS 5
#define CONFIG_ADDRESS 0

struct phoneConfig {
  unsigned char serviceFlags;
  char number[PHONE_NUMBER_LENGTH + 1];
};

struct eepromConfig {
  int temperatureAdjustment;
  int temperatureHighThreshold;
  int temperatureLowThreshold;
  int lightThreshold;
  phoneConfig registeredNumbers[MAX_PHONE_NUMBERS];
} config;


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
#define IR_SAVE_TO_EEPROM 0xF7E817    // ('Lent' on xanlite remote, for now)
// When a key is maintained pressed, a different code is sent, and we want to repeat the operation
#define IR_REPEAT_CODE 0xFFFFFFFF
unsigned long previousCode = 0;

// GSM
// initialize the library instance
#define PINNUMBER ""
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
#
#define LIGHT_PIN 0
#define TEMPERATURE_PIN 6

/* DS18B20 Temperature chip i/o */
OneWire  ds(TEMPERATURE_PIN);
#define MAX_DS1820_SENSORS 1
byte addr[MAX_DS1820_SENSORS][8];

char msgBuf[20];
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
#define MAX_SMS_LENGTH 10
boolean gsmEnabled = false;

boolean statusOK = true;

void setup(void) {
  Serial.begin(9600);
  EEPROM.setMaxAllowedWrites(10);
  lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
  print(0, 0, "Init AquaMon");
  readConfig();
  print(0, 1, "Init IR");
  reception_ir.enableIRIn(); // init receiver
  delay(250);

  print(0, 1, "DS1820 Test");
  if (!ds.search(addr[0]))
  {
    print(0, 1, "Error addr 0");
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
  print(0, 0, "GSM init.");
  while(notConnected)
  {
    print(0, 1, "Connecting");
    if(gsmAccess.begin(PINNUMBER)==GSM_READY) {
      print(0, 1, "GSM Connected");
      notConnected = false;
    } else {
      print(0, 1, "Not connected");
      delay(200);
    }
  }
  delay(500);
  lcd.clear();
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
    sendStatus("");
    statusOK = true;  // TODO : should be reset more often ? each loop ?
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
    print(0, 0, "CRC is not valid");
  } else if ( addr[sensor][0] != 0x28) {
    print(0, 0, "Not DS18B20 family.");
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

    sprintf(temperatureMsg, "Temp: %c%d.%d",signBit ? '-' : '+', whole, fract < 10 ? 0 : fract);
    print(0, 0, temperatureMsg);
    if((tc_100 < config.temperatureLowThreshold) || (tc_100 > config.temperatureHighThreshold)) {
      temperatureOK = false;
    }
  }
  return temperatureOK;
}

boolean checkLight() {
  long lightLevel = 0;
  boolean lightOK = true;
  lightLevel = analogRead(LIGHT_PIN);
  sprintf(lightMsg, "Light: %d", lightLevel);
  print(0, 1, lightMsg);
  if(lightLevel < config.lightThreshold) {
    lightOK = false;
  }
  //return(lightOK);
  return true;
}

void checkSMS() {
  char from[15];
  char c;
  int cptr = 0;
  Serial.println("Checking SMS");
  if (sms.available())
  {
    Serial.println("Msg received from:");

    // Get remote number
    sms.remoteNumber(from, 20);
    Serial.println(from);

    // An example of message disposal
    // Any messages starting with # should be discarded
    if (sms.peek() == '#') {
      Serial.println("Disc SMS");
      sms.flush();
    } else {
      // Read message bytes and print them
      while ((c = sms.read()) && (cptr < MAX_SMS_LENGTH)) {
        msgBuf[cptr++] = c;
      }
      msgBuf[cptr] = 0;

      Serial.println(msgBuf);
      if(strncmp(msgBuf, "status", 6) == 0) {
        sendStatus(from);
      }
      // Delete message from modem memory
      sms.flush();
      Serial.println("MESSAGE DELETED");
    }
  }
}

void sendStatus(char *toNumber) {
  char txtMsg[100];
  print(0, 0, "Sending SMS...");
  print(0, 1, "To: ");
  print(4, 1, toNumber);
  sprintf(txtMsg, "%s %s", temperatureMsg, lightMsg);
  Serial.println(txtMsg);

  // send the message
  if(gsmEnabled) {
    sms.beginSMS(toNumber);
    sms.print(txtMsg);
    sms.endSMS();
    print(0, 0, "SMS sent");
  }
}

void print(int col, int row, char* displayMsg) {
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

void resetIRDisplay() {
  //lcd.setCursor(14,0);
  //lcd.print("  ");
  lcd.setCursor(15,1);
  lcd.print(" ");
}

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
      sprintf(msgBuf, "Light th: %d", config.lightThreshold);
      print(0, 1, msgBuf);
    break;
    case IR_DECR_LIGHT_THRESHOLD:
      config.lightThreshold --;
      sprintf(msgBuf, "Light th: %d", config.lightThreshold);
      print(0, 1, msgBuf);
    break;

    case IR_INCR_TEMP_ADJUSTMENT:
      config.temperatureAdjustment ++;
      sprintf(msgBuf, "Temp Adj: %d", config.temperatureAdjustment);
      print(0, 1, msgBuf);
    break;
    case IR_DECR_TEMP_ADJUSTMENT:
      config.temperatureAdjustment --;
      sprintf(msgBuf, "Temp Adj: %d", config.temperatureAdjustment);
      print(0, 1, msgBuf);
    break;

    case IR_DISPLAY_THRESHOLDS:
      sprintf(msgBuf, "Temp: %d %d", config.temperatureLowThreshold, config.temperatureHighThreshold);
      print(0, 0, msgBuf);
      sprintf(msgBuf, "Light: %d", config.lightThreshold);
      print(0, 1, msgBuf);
    break;
    default:
      print(15, 1, "?");
  }
}

void readConfig() {
  print(0, 1, "Read Config");
  EEPROM.readBlock(CONFIG_ADDRESS, config);
  if(config.lightThreshold == -1) {
    config.lightThreshold = 500;
    config.temperatureAdjustment = 0;
    config.temperatureHighThreshold = 2700;
    config.temperatureLowThreshold = 2100;
    config.registeredNumbers[0].serviceFlags = 0xFF;
    strcpy(config.registeredNumbers[0].number, REMOTE_NUMBER);
    Serial.println("Saving");
    EEPROM.writeBlock(CONFIG_ADDRESS, config);
  }
}


