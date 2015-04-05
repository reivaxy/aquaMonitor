#include <avr/pgmspace.h>

// Messages and formats saved in  to save RAM
const char initAquamonMsg[] PROGMEM = {"Init AquaMon"};
#define INIT_AQUAMON_MSG 0
const char initIRMsg[] PROGMEM = {"Init IR remote"};
#define INIT_IR_MSG 1
const char tempInitMsg[] PROGMEM = {"DS1820 Test"};
#define TEMP_INIT_MSG 2
const char addrErrMsg[] PROGMEM = {"Error addr 0"};
#define ADDR_ERR_MSG 3
const char initGSMMsg[] PROGMEM = {"GSM init."};
#define INIT_GSM_MSG 4

const char connectingGSMMsg[] PROGMEM = {"GSM Connecting"};
#define CONNECTING_GSM_MSG 5
const char connectedGSMMsg[] PROGMEM = {"GSM Connected"};
#define CONNECTED_GSM_MSG 6
const char notConnectedGSMMsg[] PROGMEM = {"Not connected"};
#define NOT_CONNECTED_GSM_MSG 7
const char crcNotValidMsg[] PROGMEM = {"CRC is not valid"};
#define CRC_NOT_VALID_MSG 8
const char familyMsg[] PROGMEM = {"Not DS18B20 family."};
#define FAMILY_MSG 9
const char temperatureMsgFormat[] PROGMEM = {"Temp: %c%d.%d"};
#define TEMPERATURE_MSG_FORMAT 10
const char lightMsgFormat[] PROGMEM = {"Light: %d"};
#define LIGHT_MSG_FORMAT 11
const char checkSMSMsg[] PROGMEM = {"Checking SMS"};
#define CHECK_SMS_MSG 12
const char fromNumberMsg[] PROGMEM = {"Msg received from:"};
#define FROM_NUMBER_MSG 13
const char discardSMSMsg[] PROGMEM = {"Discard SMS"};
#define DISCARD_SMS_MSG 14
const char sendingSMSMsg[] PROGMEM = {"Sending SMS..."};
#define SENDING_SMS_MSG 15
const char SMSSentMsg[] PROGMEM = {"SMS sent"};
#define SMS_SENT_MSG 16
const char lightThresholdMsgFormat[] PROGMEM = {"Light th: %d"};
#define LIGHT_THRESHOLD_MSG_FORMAT 17
const char temperatureAdjustmentMsgFormat[] PROGMEM = {"Temp Adj: %d"};
#define TEMPERATURE_ADJUSTMENT_MSG_FORMAT 18
const char temperatureThresholdMsgFormat[] PROGMEM = {"Temp: %d %d"};
#define TEMPERATURE_THRESHOLD_MSG_FORMAT 19
const char numberSubscribedMsg[] PROGMEM = {"Sub to %s done"};
#define NUMBER_SUBSCRIBED_MSG 20
const char numberNotSubscribedMsg[] PROGMEM = {"Sub to %s failed: full"};
#define NUMBER_NOT_SUBSCRIBED_MSG 21
const char numberUnsubscribedMsg[] PROGMEM = {"Unsub to %s done"};
#define NUMBER_UNSUBSCRIBED_MSG 22
const char numberNotUnsubscribedMsg[] PROGMEM = {"Unsub to %s failed: not found"};
#define NUMBER_NOT_UNSUBSCRIBED_MSG 23
const char savingConfigMsg[] PROGMEM = {"Saving config"};
#define SAVING_CONFIG_MSG 24
const char configSavedMsg[] PROGMEM = {"Config saved"};
#define CONFIG_SAVED_MSG 25
const char readingConfigMsg[] PROGMEM = {"Reading config"};
#define READING_CONFIG_MSG 26
const char accessDeniedMsg[] PROGMEM = {"Access denied"};
#define ACCESS_DENIED_MSG 27
const char unknownMsg[] PROGMEM = {"Unknown message"};
#define UNKNOWN_MSG 28
const char buildMsg[] PROGMEM = {"aquaMonitor "__DATE__ " " __TIME__};
#define BUILD_MSG 29
const char resetSubDoneMsg[] PROGMEM = {"Reset sub. done"};
#define RESET_SUB_DONE_MSG 30
const char lightScheduleMsgFormat[] PROGMEM = {"Light schedule %d:%d - %d:%d"};
#define LIGHT_SCHEDULE_MSG_FORMAT 31
const char temperatureAdjustmentSetMsg[] PROGMEM = {"Temp adjustment set"};
#define TEMPERATURE_ADJUSTMENT_SET_MSG 32
const char temperatureThresholdsSetMsg[] PROGMEM = {"Temp thresholds set"};
#define TEMPERATURE_THRESHOLDS_SET_MSG 33
const char lightThresholdSetMsg[] PROGMEM = {"Light threshold set"};
#define LIGHT_THRESHOLD_SET_MSG 34

const char* const messages[] PROGMEM = {
   initAquamonMsg, initIRMsg, tempInitMsg, addrErrMsg, initGSMMsg,   // 0 to 4
   connectingGSMMsg, connectedGSMMsg, notConnectedGSMMsg, crcNotValidMsg, familyMsg,  // 5 to 9
   temperatureMsgFormat, lightMsgFormat, checkSMSMsg, fromNumberMsg, discardSMSMsg,   // 10 TO 14
   sendingSMSMsg, SMSSentMsg, lightThresholdMsgFormat, temperatureAdjustmentMsgFormat, temperatureThresholdMsgFormat,  // 15 TO 19
   numberSubscribedMsg, numberNotSubscribedMsg, numberUnsubscribedMsg, numberNotUnsubscribedMsg, savingConfigMsg,      // 20 TO 24
   configSavedMsg, readingConfigMsg, accessDeniedMsg, unknownMsg, buildMsg,    // 25 TO 29
   resetSubDoneMsg, lightScheduleMsgFormat, temperatureAdjustmentSetMsg, temperatureThresholdsSetMsg, lightThresholdSetMsg
};

