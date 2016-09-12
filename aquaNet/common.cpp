#include "common.h"

void checkSerial(HardwareSerial *serial, char *message, void (*processMsgFunc)(char *)) {
  char incomingChar;
  int length;

  while (serial->available() > 0) {
    incomingChar = serial->read();
    if(incomingChar > 0) {
      if((incomingChar == '\r') || (incomingChar == '\n')) {
        processMsgFunc(message);
        message[0] = 0;
      } else {
        length = strlen(message);
        if(length < MAX_SERIAL_INPUT_MESSAGE - 2) {
          message[length] = incomingChar;
          message[length + 1] = 0;
        } else {
          // Ignore  message, for now
          message[0] = 0;
        }
      }
    }
  }
}
