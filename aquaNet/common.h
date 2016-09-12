#include <HardwareSerial.h>
#define MAX_SERIAL_INPUT_MESSAGE 161

void checkSerial(HardwareSerial *serial, char *message, void (*processMsgFunc)(char *));