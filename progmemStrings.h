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
const char SMSSentMsg[] PROGMEM = {"SMS sent"};
const char lightThresholdMsgFormat[] PROGMEM = {"Light th: %d"};
const char temperatureAdjustmentMsgFormat[] PROGMEM = {"Temp Adj: %d"};
const char temperatureThresholdMsgFormat[] PROGMEM = {"Temp: %d %d"};
const char numberSubscribedMsg[] PROGMEM = {"Sub to %s done"};
const char numberNotSubscribedMsg[] PROGMEM = {"Sub to %s failed: full"};
const char numberUnsubscribedMsg[] PROGMEM = {"Unsub to %s done"};
const char numberNotUnsubscribedMsg[] PROGMEM = {"Unsub to %s failed: not found"};
const char savingConfigMsg[] PROGMEM = {"Saving config"};
const char configSavedMsg[] PROGMEM = {"Config saved"};
const char readingConfigMsg[] PROGMEM = {"Reading config"};


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
#define SMS_SENT_MSG 16
#define LIGHT_THRESHOLD_MSG_FORMAT 17
#define TEMPERATURE_ADJUSTMENT_MSG_FORMAT 18
#define TEMPERATURE_THRESHOLD_MSG_FORMAT 19
#define NUMBER_SUBSCRIBED_MSG 20
#define NUMBER_NOT_SUBSCRIBED_MSG 21
#define NUMBER_UNSUBSCRIBED_MSG 22
#define NUMBER_NOT_UNSUBSCRIBED_MSG 23
#define SAVING_CONFIG_MSG 24
#define CONFIG_SAVED_MSG 25
#define READING_CONFIG_MSG 26

const char* const messages[] PROGMEM = {initAquamonMsg, initIRMsg, tempInitMsg, addrErrMsg, initGSMMsg, connectingGSMMsg,
   connectedGSMMsg, notConnectedGSMMsg, crcNotValidMsg, familyMsg, temperatureMsgFormat, lightMsgFormat, checkSMSMsg,
   fromNumberMsg, discardSMSMsg, sendingSMSMsg, SMSSentMsg, lightThresholdMsgFormat, temperatureAdjustmentMsgFormat,
   temperatureThresholdMsgFormat, numberSubscribedMsg, numberNotSubscribedMsg, numberUnsubscribedMsg, numberNotUnsubscribedMsg,
   savingConfigMsg, configSavedMsg, readingConfigMsg,
};

