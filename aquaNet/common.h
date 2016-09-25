#include <HardwareSerial.h>
#define MAX_SERIAL_INPUT_MESSAGE 600 // some "big" Json strings

boolean readFromSerial(HardwareSerial *serial, char *message, unsigned long timeOut);

boolean writeToSerial(HardwareSerial *serial, char *message, unsigned long timeOut);
