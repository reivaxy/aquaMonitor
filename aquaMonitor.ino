#include <OneWire.h>
#include <LiquidCrystal.h>
#include <GSM.h>
#include <IRremote.h>

// TODO : move variable declaration, it's a mess ! (use C++ ?)
// TODO : handle SMS better (limit then, centralize handling...)
// TODO : centralize message LCD displaying (transient IR msgs...) to insure a minimum display time for all
// IR behavior very much depends on remote control key layout...

// Init IR
#define IR_PIN 2
IRrecv reception_ir(IR_PIN);
decode_results code;
#define IR_DISPLAY_PHONE_NUMBER 0xF720DF       // (R key on xanlite remote, for now)
#define IR_INCR_LIGHT_THRESHOLD 0xF700FF       // (light+ on xanlite remote, for now)
#define IR_DECR_LIGHT_THRESHOLD 0xF7807F       // (light- on xanlite remote, for now)
#
#define IR_INCR_TEMP_ADJUSTMENT 0xF740BF       // (OFF on xanlite remote, for now)
#define IR_DECR_TEMP_ADJUSTMENT 0xF7C03F       // (ON on xanlite remote, for now)
#
#define IR_DISPLAY_THRESHOLDS   0xF7A05F       // (V on xanlite remote, for now)
#
#
#define IR_SAVE_TO_EEPROM 0xF7E817    // (Lent on xanlite remote, for now)

// When a key is maintained pressed, a different code is sent, and we want to repeat the operation
#define IR_REPEAT_CODE 0xFFFFFFFF
unsigned long previousCode = 0;

// TODO : read these from EEPROM (and initialize them if not found
long lightThreshold = 500;
int temperatureAdjustment = 0;

#define TEMPERATURE_LOW_THRESHOLD 24
#define TEMPERATURE_HIGH_THRESHOLD 27

// GSM
// initialize the library instance
#define PINNUMBER ""
GSM gsmAccess; // include a 'true' parameter for debug enabled
GSM_SMS sms;

// defined in a private file haha !
#include <remoteNumber.h>
char remoteNumber[11] = REMOTE_NUMBER;  // TODO: make it configurable & saved in Flash ?

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

char strBuf[20];

#define TEMPERATURE_CHECK_PERIOD 5000
unsigned long lastTemperatureCheck = millis() - TEMPERATURE_CHECK_PERIOD;

#define LIGHT_CHECK_PERIOD 5000
unsigned long lastLightCheck = millis() - LIGHT_CHECK_PERIOD;

#define IR_DISPLAY_TIMEOUT 250
unsigned long lastIrDisplay = 0;

boolean gsmEnabled = false;

void setup(void) {
  Serial.begin(9600);

  lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
  print(0, 0, (char *)F("Init AquaMon"));
  print(0, 1, (char *)F("Init IR"));
  reception_ir.enableIRIn(); // init receiver
  delay(250);

  print(0, 1, (char *)F("DS1820 Test"));
  if (!ds.search(addr[0]))
  {
    print(0, 1, (char *)F("Error addr 0"));
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
  print(0, 0, (char *)F("GSM init."));
  while(notConnected)
  {
    print(0, 1, (char *)F("Connecting"));
    if(gsmAccess.begin(PINNUMBER)==GSM_READY) {
      print(0, 1, (char *)F("GSM Connected"));
      notConnected = false;
    } else {
      print(0, 1, (char *)F("Not connected"));
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
    checkLight();
    lastLightCheck = millis();
  }
  if((now - lastTemperatureCheck) >= TEMPERATURE_CHECK_PERIOD) {
    checkTemperature();
    lastTemperatureCheck = millis();
  }
  if((now - lastIrDisplay) >= IR_DISPLAY_TIMEOUT) {
    resetIRDisplay();
  }
}

void checkTemperature() {
  int highByte, lowByte, tReading, signBit, tc_100;
  float temperature;
  byte sensor = 0;

  byte i;
  byte present = 0;
  byte data[12];

  if ( OneWire::crc8( addr[sensor], 7) != addr[sensor][7]) {
    print(0, 0, (char *)F("CRC is not valid"));
  } else if ( addr[sensor][0] != 0x28) {
    print(0, 0, (char *)F("Not DS18B20 family."));
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
    //tc_100 += temperatureAdjustment; 

    //temperature = tc_100 / 100;
    //sprintf(strBuf, (char *)F("Temp :%c0.2%f"),signBit ? '-' : '+', temperature);
    //print(0, 0, strBuf);
    // TODO : centralize SMS handling to limit them
    //if((temperature < TEMPERATURE_LOW_THRESHOLD) || (temperature > TEMPERATURE_HIGH_THRESHOLD)) {
      //sendSMS(strBuf);
    //}
  }
}

void checkLight() {
  long lightLevel = 0;

  lightLevel = analogRead(LIGHT_PIN);
  sprintf(strBuf, (char *)F("Light: %d"), lightLevel);
  print(0, 1, strBuf);
  if(lightLevel < lightThreshold) {
    sendSMS(strBuf);
  }
}

void sendSMS(char *txtMsg){
  print(0, 0, (char *)F("Sending SMS..."));
  print(0, 1, (char *)F("To:"));
  print(4, 1, remoteNumber);
  Serial.println(txtMsg);

  // send the message
  if(gsmEnabled) {
    sms.beginSMS(remoteNumber);
    sms.print(txtMsg);
    sms.endSMS();
    print(0, 0, (char *)F("SMS sent"));
  }
}

void print(int col, int row, char* displayMsg) {
  char msg[MSG_MAX_LENGTH + 1];
  Serial.println(displayMsg);
  strncpy(msg, displayMsg, MSG_MAX_LENGTH);
  while(strlen(msg) < MSG_MAX_LENGTH) {
    strcat(msg, " ");
  }
  lcd.setCursor(col, row);
  lcd.print(msg);
}

void resetIRDisplay() {
  lcd.setCursor(14,0);
  lcd.print("  ");
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
  print(14, 0, "ir");
  switch(irCode) {
    case IR_DISPLAY_PHONE_NUMBER:
      print(0, 0, remoteNumber);
    break;
    case IR_INCR_LIGHT_THRESHOLD:
      lightThreshold ++;
      sprintf(strBuf, (char *)F("Light th: %d"), lightThreshold);
      print(0, 1, strBuf);
    break;
    case IR_DECR_LIGHT_THRESHOLD:
      lightThreshold --;
      sprintf(strBuf, (char *)F("Light th: %d"), lightThreshold);
      print(0, 1, strBuf);
    break;

    case IR_INCR_TEMP_ADJUSTMENT:
      temperatureAdjustment ++;
      sprintf(strBuf, (char *)F("Temp Adjust: %d"), temperatureAdjustment);
      print(0, 1, strBuf);
    break;
    case IR_DECR_TEMP_ADJUSTMENT:
      lightThreshold --;
      sprintf(strBuf, (char *)F("Temp Adjust: %d"), temperatureAdjustment);
      print(0, 0, strBuf);
    break;

    case IR_DISPLAY_THRESHOLDS:
      sprintf(strBuf, (char *)F("Temp set: %d %d"), TEMPERATURE_LOW_THRESHOLD, TEMPERATURE_HIGH_THRESHOLD);
      print(0, 0, strBuf);
      sprintf(strBuf, (char *)F("Light set: %d"), lightThreshold);
      print(0, 1, strBuf);
    break;
    default:
      print(15, 1, "?");
  }
}
