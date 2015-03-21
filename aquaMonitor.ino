#include <OneWire.h>
#include <LiquidCrystal.h>
#include <GSM.h>

#define PINNUMBER ""

// GSM=======================================================
// initialize the library instance
GSM gsmAccess(true); // include a 'true' parameter for debug enabled
GSM_SMS sms;

// char array of the telephone number to send SMS
char remoteNumber[20]= "xx";  

// LCD=======================================================
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11 , 12);

#define LCD_WIDTH 20
#define LCD_HEIGHT 2

int lightPin = 0;

/* DS18B20 Temperature chip i/o */

OneWire  ds(6);  // on pin 6
#define MAX_DS1820_SENSORS 2
byte addr[MAX_DS1820_SENSORS][8];
void setup(void) 
{
  Serial.begin(9600);
  lcd.begin(LCD_WIDTH, LCD_HEIGHT,1);
  print(0, 0, "Init AquaMon");
  print(0, 1, "DS1820 Test");
  if (!ds.search(addr[0])) 
  {
    print(0, 1, "No more addr 0");
    ds.reset_search();
    delay(250);
  }
  /*
  if ( !ds.search(addr[1])) 
  {
    print(0, 1, "No more addr 1");
    ds.reset_search();
    delay(250);
  }
  */
  delay(7000);
  // GSM connection state
  boolean notConnected = true;

  // Start GSM shield
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  clear();
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
  clear();
}

int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
char buf[20];
char smsMsg[160];
byte sensor = 0;

void loop(void) 
{
  byte i;
  byte present = 0;
  byte data[12];
  long lightLevel = 0;
  smsMsg[0] = 0;

  if ( OneWire::crc8( addr[sensor], 7) != addr[sensor][7]) {
    print(0, 0, "CRC is not valid");
  } else if ( addr[sensor][0] != 0x28) {
    print(0, 0, "Device is not a DS18B20 family device.");
  } else {
    ds.reset();
    ds.select(addr[sensor]);
    ds.write(0x44,1);         // start conversion, with parasite power on at the end

    delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.

    present = ds.reset();
    ds.select(addr[sensor]);    
    ds.write(0xBE);         // Read Scratchpad

    for ( i = 0; i < 9; i++) 
    {           // we need 9 bytes
      data[i] = ds.read();
    }

    LowByte = data[0];
    HighByte = data[1];
    TReading = (HighByte << 8) + LowByte;
    SignBit = TReading & 0x8000;  // test most sig bit
    if (SignBit) // negative
    {
      TReading = (TReading ^ 0xffff) + 1; // 2's comp
    }
    Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25

    Whole = Tc_100 / 100;  // separate off the whole and fractional portions
    Fract = Tc_100 % 100;

    sprintf(buf, "Temp :%c%d.%d",SignBit ? '-' : '+', Whole, Fract < 10 ? 0 : Fract);
    print(0, 0, buf);
    strcpy(smsMsg, buf);
  }

  lightLevel = analogRead(lightPin);
  sprintf(buf, "Light: %d", lightLevel);
  print(0, 1, buf);
  sprintf(smsMsg, "%s, %s\n", smsMsg, buf);

  if((Whole > 29) || (lightLevel < 600)) {
    sendSMS('sending');
    sendSMS(smsMsg);
  }
  delay(1000);
}

void sendSMS(char *txtMsg){
  print(0, 0, "Sending SMS");
  Serial.println(txtMsg);

  // send the message
  sms.beginSMS(remoteNumber);
  sms.print(txtMsg);
  sms.endSMS(); 
  print(0, 0, "SMS sent...");
}

void print(int col, int row, char* displayMsg) {
  char msg[25];
  Serial.println(displayMsg);
  strncpy(msg, displayMsg, 20);
  while(strlen(msg) < 20) {
    strcat(msg, " ");
  }
  lcd.setCursor(col, row);
  lcd.print(msg);
}

void clear() {
  lcd.clear();
}
