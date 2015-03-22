#include <OneWire.h>
#include <LiquidCrystal.h>
#include <GSM.h>
#define PINNUMBER ""
#include <IRremote.h>


// TODO : move variable declaration, it's a mess ! (use C++ ?)
// TODO : handle SMS better (limit then, centralize handling...)
// TODO : centralize message LCD displaying (transient IR msgs...) to insure a minimum display time for all
// IR behavior very much depends on remote control key layout...

// Init IR
int inPin = 2;
IRrecv reception_ir(inPin);
decode_results code;
#define IR_DISPLAY_PHONE_NUMBER 0xF720DF       // (R key on xanlite remote, for now)
#define IR_INCR_LIGHT_THRESHOLD 0xF700FF       // (light+ on xanlite remote, for now)
#define IR_DECR_LIGHT_THRESHOLD 0xF7807F       // (light- on xanlite remote, for now)
#define IR_DISPLAY_LIGHT_THRESHOLD 0xF740BF    // (OFF on xanlite remote, for now)
#define IR_DISPLAY_TEMPERATURE_THRESHOLD 0xF7C03F    // (ON on xanlite remote, for now)

// When a key is maintained pressed, a different code is sent, and we want to repeat the operation
#define IR_REPEAT_CODE 0xFFFFFFFF
unsigned long previousCode = 0;

long lightThreshold = 500;

// GSM
// initialize the library instance
GSM gsmAccess(true); // include a 'true' parameter for debug enabled
GSM_SMS sms;

// defined in a private file haha !
#include <remoteNumber.h>
char remoteNumber[20]= REMOTE_NUMBER;  // TODO: make it configurable & saved on SD card

// LCD
// initialize the library with the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11 , 12);

#define LCD_WIDTH 20
#define LCD_HEIGHT 2

int lightPin = 0;

/* DS18B20 Temperature chip i/o */
OneWire  ds(6);  // on pin 6
#define MAX_DS1820_SENSORS 2
byte addr[MAX_DS1820_SENSORS][8];
long temperatureThreshold = 26;

char buf[20];
char smsMsg[160];

unsigned long temperatureCheckPeriod = 5000;
unsigned long lastTemperatureCheck = millis() - temperatureCheckPeriod;

unsigned long lightCheckPeriod = 5000;
unsigned long lastLightCheck = millis() - lightCheckPeriod;

unsigned long irDisplayTimeOut = 250;
unsigned long lastIrDisplay = 0;

boolean gsmEnabled = false;

void setup(void) {
  Serial.begin(9600);

  lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
  print(0, 0, "Init AquaMon");
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

  if((now - lastLightCheck) >= lightCheckPeriod) {
    checkLight();
    lastLightCheck = millis();
  }
  if((now - lastTemperatureCheck) >= temperatureCheckPeriod) {
    checkTemperature();
    lastTemperatureCheck = millis();
  }
  if((now - lastIrDisplay) >= irDisplayTimeOut) {
    resetIRDisplay();
  }
}

void checkTemperature() {
  int highByte, lowByte, tReading, signBit, tc_100, whole, fract;
  byte sensor = 0;

  byte i;
  byte present = 0;
  byte data[12];

  if ( OneWire::crc8( addr[sensor], 7) != addr[sensor][7]) {
    print(0, 0, "CRC is not valid");
  } else if ( addr[sensor][0] != 0x28) {
    print(0, 0, "Device is not a DS18B20 family device.");
  } else {
    ds.reset();
    ds.select(addr[sensor]);
    ds.write(0x44,1);         // start conversion, with parasite power on at the end

    //delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr[sensor]);
    ds.write(0xBE);         // Read Scratchpad

    for ( i = 0; i < 9; i++)
    {           // we need 9 bytes
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

    whole = tc_100 / 100;  // separate off the whole and fractional portions
    fract = tc_100 % 100;

    sprintf(buf, "Temp :%c%d.%d",signBit ? '-' : '+', whole, fract < 10 ? 0 : fract);
    print(0, 0, buf);
    // TODO : centralize SMS handling to limit them
    if(whole > temperatureThreshold) {
      sendSMS(buf);
    }
  }
}

void checkLight() {
  long lightLevel = 0;

  lightLevel = analogRead(lightPin);
  sprintf(buf, "Light: %d", lightLevel);
  print(0, 1, buf);
  if(lightLevel < lightThreshold) {
    sendSMS(buf);
  }
}

void sendSMS(char *txtMsg){
  print(0, 0, "Sending SMS...");
  print(0, 1, "To:");
  print(4, 1, remoteNumber);
  Serial.println(txtMsg);

  // send the message
  if(gsmEnabled) {
    sms.beginSMS(remoteNumber);
    sms.print(txtMsg);
    sms.endSMS();
    print(0, 0, "SMS sent");
  }
}

void print(int col, int row, char* displayMsg) {
  char msg[25];
  int availLength = 14;  // last 2 are for IR marks
  Serial.println(displayMsg);
  strncpy(msg, displayMsg, availLength);
  while(strlen(msg) < availLength) {
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
  char strBuf[20];
  Serial.println(irCode, HEX);
  print(14, 0, "ir");
  switch(irCode) {
    case IR_DISPLAY_PHONE_NUMBER:
      print(0, 0, remoteNumber);
    break;
    case IR_INCR_LIGHT_THRESHOLD:
      lightThreshold ++;
      sprintf(strBuf, "Light th: %d", lightThreshold);
      print(0, 1, strBuf);
    break;
    case IR_DECR_LIGHT_THRESHOLD:
      lightThreshold --;
      sprintf(strBuf, "Light th: %d", lightThreshold);
      print(0, 1, strBuf);
    break;
    case IR_DISPLAY_LIGHT_THRESHOLD:
      sprintf(strBuf, "Light th: %d", lightThreshold);
      print(0, 1, strBuf);
    break;
    case IR_DISPLAY_TEMPERATURE_THRESHOLD:
      sprintf(strBuf, "Temp th: %d", temperatureThreshold);
      print(0, 0, strBuf);
    break;
    default:
      print(15, 1, "?");
  }
}