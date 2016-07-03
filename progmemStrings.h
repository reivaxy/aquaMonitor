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
const char lightThresholdMsgFormat[] PROGMEM = {"Light limit: %d"};
#define LIGHT_THRESHOLD_MSG_FORMAT 17
const char temperatureAdjustmentMsgFormat[] PROGMEM = {"Temp Adj: %d"};
#define TEMPERATURE_ADJUSTMENT_MSG_FORMAT 18
const char temperatureThresholdMsgFormat[] PROGMEM = {"Temp : %d %d"};
#define TEMPERATURE_THRESHOLD_MSG_FORMAT 19
const char numberSubscribedMsg[] PROGMEM = {"Sub to %s done"};
#define NUMBER_SUBSCRIBED_MSG 20
const char numberNotSubscribedMsg[] PROGMEM = {"Sub to %s failed"};
#define NUMBER_NOT_SUBSCRIBED_MSG 21
const char numberUnsubscribedMsg[] PROGMEM = {"Unsub to %s done"};
#define NUMBER_UNSUBSCRIBED_MSG 22
const char numberNotUnsubscribedMsg[] PROGMEM = {"Unsub to %s failed: not found"};
#define NUMBER_NOT_UNSUBSCRIBED_MSG 23
const char savingConfigMsg[] PROGMEM = {"Saving config"};
#define SAVING_CONFIG_MSG 24
const char configSavedMsg[] PROGMEM = {"Config saved"};
#define CONFIG_SAVED_MSG 25
const char newConfigMsg[] PROGMEM = {"New config"};
#define NEW_CONFIG_MSG 26
const char accessDeniedMsg[] PROGMEM = {"Access denied"};
#define ACCESS_DENIED_MSG 27
const char unknownMsg[] PROGMEM = {"What ???"};
#define UNKNOWN_MSG 28
const char buildMsg[] PROGMEM = {"aquaMonitor "__DATE__ " " __TIME__};
#define BUILD_MSG 29
const char resetSubDoneMsg[] PROGMEM = {"Reset sub. done (save ?)"};
#define RESET_SUB_DONE_MSG 30
const char setAdminDoneMsg[] PROGMEM = {"Admin set (save ?)"};
#define SET_ADMIN_DONE_MSG 31
const char lightScheduleMsgFormat[] PROGMEM = {"Light schedule %3d:%02d - %02d:%02d"};
#define LIGHT_SCHEDULE_MSG_FORMAT 32
const char temperatureAdjustmentSetMsg[] PROGMEM = {"Temp adjustment set (save ?)"};
#define TEMPERATURE_ADJUSTMENT_SET_MSG 33
const char temperatureThresholdsSetMsg[] PROGMEM = {"Temp thresholds set (save ?)"};
#define TEMPERATURE_THRESHOLDS_SET_MSG 34
const char lightThresholdSetMsg[] PROGMEM = {"Light threshold set (save ?)"};
#define LIGHT_THRESHOLD_SET_MSG 35
const char intervalSetMsg[] PROGMEM = {"Interval set (save ?)"};
#define INTERVAL_SET_MSG 36
const char unknownNumberMsg[] PROGMEM = {"Unknown number"};
#define UNKNOWN_NUMBER_MSG 37
const char currentDateFormatMsg[] PROGMEM = {"Date: %3d/%02d/%02d %02d:%02d"};
#define CURRENT_DATE_FORMAT_MSG 38
const char lightScheduleSetMsg[] PROGMEM = {"Light schedule set (save ?)"};
#define LIGHT_SCHEDULE_SET_MSG 39
const char temperatureAlertMsg[] PROGMEM = {"Temp Alert"};
#define TEMPERATURE_ALERT_MSG 40
const char lightAlertMsg[] PROGMEM = {"Light alert"};
#define LIGHT_ALERT_MSG 41
const char levelAlertMsg[] PROGMEM = {"Level alert"};
#define LEVEL_ALERT_MSG 42
const char levelLowMsg[] PROGMEM = {"Level: low"};
#define LEVEL_LOW_MSG 43
const char levelHighMsg[] PROGMEM = {"Level: high"};
#define LEVEL_HIGH_MSG 44

const char inSmsInterval[] PROGMEM = {"interval "};
#define IN_SMS_INTERVAL 45
const char inSmsTempAdj[] PROGMEM = {"temp adj "};
#define IN_SMS_TEMP_ADJ 46
const char inSmsConfig[] PROGMEM = {"config"};
#define IN_SMS_CONFIG 47
const char inSmsTemp[] PROGMEM = {"temp "};
#define IN_SMS_TEMP 48
const char inSmsLight[] PROGMEM = {"light "};
#define IN_SMS_LIGHT 49
const char inSmsSchedule[] PROGMEM = {"schedule "};
#define IN_SMS_SCHEDULE 50
const char inSmsSave[] PROGMEM = {"save"};
#define IN_SMS_SAVE 51
const char inSmsStatus[] PROGMEM = {"status"};
#define IN_SMS_STATUS 52
const char inSmsSub[] PROGMEM = {"sub "};
#define IN_SMS_SUB 53
const char inSmsUnsub[] PROGMEM = {"unsub "};
#define IN_SMS_UNSUB 54
const char inSmsResetSub[] PROGMEM = {"reset sub"};
#define IN_SMS_RESET_SUB 55
const char inSmsSubs[] PROGMEM = {"subs"};
#define IN_SMS_SUBS 56
const char inSmsResetLCD[] PROGMEM = {"display"};
#define IN_SMS_RESET_LCD 57
const char inSmsSetAdmin[] PROGMEM = {"admin"};
#define IN_SMS_SET_ADMIN 58

const char inSmsIntervalFormat[] PROGMEM = {"interval %ld"};
#define IN_SMS_INTERVAL_FORMAT 59
const char inSmsTempAdjFormat[] PROGMEM = {"temp adj %d"};
#define IN_SMS_TEMP_ADJ_FORMAT 60
const char inSmsTempFormat[] PROGMEM = {"temp %d %d"};
#define IN_SMS_TEMP_FORMAT 61
const char inSmsLightFormat[] PROGMEM = {"light %d"};
#define IN_SMS_LIGHT_FORMAT 62
const char inSmsScheduleFormat[] PROGMEM = {"schedule %d:%d - %d:%d"};
#define IN_SMS_SCHEDULE_FORMAT 63
const char inSmsSubFormat[] PROGMEM = {"sub %s"};
#define IN_SMS_SUB_FORMAT 64
const char inSmsUnsubFormat[] PROGMEM = {"unsub %s"};
#define IN_SMS_UNSUB_FORMAT 65
const char inSmsSetAdminFormat[] PROGMEM = {"admin %s"};
#define IN_SMS_SET_ADMIN_FORMAT 66

const char* const messages[] PROGMEM = {
  initAquamonMsg, initIRMsg, tempInitMsg, addrErrMsg, initGSMMsg,
  connectingGSMMsg, connectedGSMMsg, notConnectedGSMMsg, crcNotValidMsg, familyMsg,
  temperatureMsgFormat, lightMsgFormat, checkSMSMsg, fromNumberMsg, discardSMSMsg,
  sendingSMSMsg, SMSSentMsg, lightThresholdMsgFormat, temperatureAdjustmentMsgFormat, temperatureThresholdMsgFormat,
  numberSubscribedMsg, numberNotSubscribedMsg, numberUnsubscribedMsg, numberNotUnsubscribedMsg, savingConfigMsg,
  configSavedMsg, newConfigMsg, accessDeniedMsg, unknownMsg, buildMsg,
  resetSubDoneMsg, setAdminDoneMsg, lightScheduleMsgFormat, temperatureAdjustmentSetMsg,
  temperatureThresholdsSetMsg, lightThresholdSetMsg,
  intervalSetMsg, unknownNumberMsg, currentDateFormatMsg, lightScheduleSetMsg, temperatureAlertMsg,
  lightAlertMsg, levelAlertMsg, levelLowMsg, levelHighMsg,

  inSmsInterval, inSmsTempAdj, inSmsConfig, inSmsTemp, inSmsLight,
  inSmsSchedule, inSmsSave, inSmsStatus, inSmsSub, inSmsUnsub,
  inSmsResetSub, inSmsSubs, inSmsResetLCD, inSmsSetAdmin,

  inSmsIntervalFormat, inSmsTempAdjFormat, inSmsTempFormat, inSmsLightFormat, inSmsScheduleFormat,
  inSmsSubFormat, inSmsUnsubFormat, inSmsSetAdminFormat



};



