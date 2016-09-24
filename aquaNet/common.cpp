#include <Arduino.h> // To get millis()
#include "common.h"

// Serial has a ~60 bytes buffer. If messages longer than 60 bytes are not read fast enough, they can be lost.
boolean readFromSerial(HardwareSerial *serial, char *message, unsigned long timeOut) {
  int incomingChar;
  int length;
  unsigned long now = millis();

  while (true) {
    incomingChar = serial->read();
    if(incomingChar > 0) {
      if((incomingChar == '\r') || (incomingChar == '\n')) {
        if(strlen(message) > 1) { // do not process the extra \n or \r when println was used
          return true;
        }
        message[0] = 0;
        return false; // Message should not be processed
      } else {
        length = strlen(message);
        if(length < MAX_SERIAL_INPUT_MESSAGE - 2) {
          message[length] = incomingChar;
          message[length + 1] = 0;
        } else {
          // Ignore  message
          message[0] = 0;
          Serial.println("Serial message too big");
          return false;
        }
      }
    } else {
      if((millis() - now) > timeOut) {
        // Keep what was read, we'll get the rest at next call... ?
        //Serial.println("Time out reading on serial");
        return false;
      }
    }
  }
}

// Write to serial, insuring message is read to not overload the buffer
boolean writeToSerial(HardwareSerial *serial, char *message, long int timeOut) {
  char *charPtr;
  int length;
  unsigned long now = millis();
  charPtr = message;
  while(*charPtr) {
    if(serial->availableForWrite()) {
      serial->write(charPtr++, 1);
    } else {
      if((millis() - now) > timeOut) {
        //Serial.println("Time out writing on serial");
        return false;
      }
    }
  }
  serial->write(0x10); // line feed to end message
}