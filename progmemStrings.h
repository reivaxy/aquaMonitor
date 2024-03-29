// DO NOT EDIT THIS FILE. It was generated from messages_en.str by running the command 'node build.js en'

#include <avr/pgmspace.h>

const char accessDeniedMsg[] PROGMEM = {"Access denied"};
#define ACCESS_DENIED_MSG 0
const char addrErrMsg[] PROGMEM = {"Error addr 0"};
#define ADDR_ERR_MSG 1
const char alertMsgFormat[] PROGMEM = {"ALERT - %s: %s, %s, %s, %s. "};
#define ALERT_MSG_FORMAT 2
const char buildMsg[] PROGMEM = {"aquaMonitor " __DATE__ " " __TIME__};
#define BUILD_MSG 3
const char checkSmsMsg[] PROGMEM = {"Checking SMS"};
#define CHECK_SMS_MSG 4
const char clearAlertDoneMsg[] PROGMEM = {"Clear alert done"};
#define CLEAR_ALERT_DONE_MSG 5
const char configResetMsg[] PROGMEM = {"Config reset"};
#define CONFIG_RESET_MSG 6
const char configSavedMsg[] PROGMEM = {"Config saved"};
#define CONFIG_SAVED_MSG 7
const char connectedGsmMsg[] PROGMEM = {"GSM Connected"};
#define CONNECTED_GSM_MSG 8
const char connectingGsmMsg[] PROGMEM = {"GSM Connecting"};
#define CONNECTING_GSM_MSG 9
const char crcNotValidMsg[] PROGMEM = {"CRC is not valid"};
#define CRC_NOT_VALID_MSG 10
const char currentDateFormatMsg[] PROGMEM = {"Date: %4d/%02d/%02d %02d:%02d"};
#define CURRENT_DATE_FORMAT_MSG 11
const char discardSmsMsg[] PROGMEM = {"Discard SMS"};
#define DISCARD_SMS_MSG 12
const char familyMsg[] PROGMEM = {"Not DS18B20 family."};
#define FAMILY_MSG 13
const char fromNumberMsg[] PROGMEM = {"Msg received from:"};
#define FROM_NUMBER_MSG 14
const char initAquamonMsg[] PROGMEM = {"Init AquaMon"};
#define INIT_AQUAMON_MSG 15
const char initGsmMsg[] PROGMEM = {"GSM init."};
#define INIT_GSM_MSG 16
const char intervalSetMsg[] PROGMEM = {"Interval set (save ?)"};
#define INTERVAL_SET_MSG 17
const char inSmsAbout[] PROGMEM = {"about"};
#define IN_SMS_ABOUT 18
const char inSmsClearAlert[] PROGMEM = {"clear"};
#define IN_SMS_CLEAR_ALERT 19
const char inSmsConfig[] PROGMEM = {"config"};
#define IN_SMS_CONFIG 20
const char inSmsInterval[] PROGMEM = {"interval "};
#define IN_SMS_INTERVAL 21
const char inSmsIntervalFormat[] PROGMEM = {"interval %ld"};
#define IN_SMS_INTERVAL_FORMAT 22
const char inSmsLightSchedule[] PROGMEM = {"light schedule "};
#define IN_SMS_LIGHT_SCHEDULE 23
const char inSmsLightScheduleFormat[] PROGMEM = {"light schedule %s %d:%d-%d:%d"};
#define IN_SMS_LIGHT_SCHEDULE_FORMAT 24
const char inSmsLightThreshold[] PROGMEM = {"light limits "};
#define IN_SMS_LIGHT_THRESHOLD 25
const char inSmsLightThresholdFormat[] PROGMEM = {"light limits %s %d-%d"};
#define IN_SMS_LIGHT_THRESHOLD_FORMAT 26
const char inSmsResetConfig[] PROGMEM = {"reset config"};
#define IN_SMS_RESET_CONFIG 27
const char inSmsResetLcd[] PROGMEM = {"display"};
#define IN_SMS_RESET_LCD 28
const char inSmsResetSub[] PROGMEM = {"reset sub"};
#define IN_SMS_RESET_SUB 29
const char inSmsSave[] PROGMEM = {"save"};
#define IN_SMS_SAVE 30
const char inSmsSetAdmin[] PROGMEM = {"admin"};
#define IN_SMS_SET_ADMIN 31
const char inSmsSetAdminFormat[] PROGMEM = {"admin %s"};
#define IN_SMS_SET_ADMIN_FORMAT 32
const char inSmsSetTime[] PROGMEM = {"time"};
#define IN_SMS_SET_TIME 33
const char inSmsSetTimeFormat[] PROGMEM = {"time %4d/%02d/%02d %02d:%02d"};
#define IN_SMS_SET_TIME_FORMAT 34
const char inSmsStatus[] PROGMEM = {"status"};
#define IN_SMS_STATUS 35
const char inSmsSub[] PROGMEM = {"sub "};
#define IN_SMS_SUB 36
const char inSmsSubs[] PROGMEM = {"subs"};
#define IN_SMS_SUBS 37
const char inSmsSubFormat[] PROGMEM = {"sub %s"};
#define IN_SMS_SUB_FORMAT 38
const char inSmsTemp[] PROGMEM = {"temp "};
#define IN_SMS_TEMP 39
const char inSmsTempAdj[] PROGMEM = {"temp adj "};
#define IN_SMS_TEMP_ADJ 40
const char inSmsTempAdjFormat[] PROGMEM = {"temp adj %d"};
#define IN_SMS_TEMP_ADJ_FORMAT 41
const char inSmsTempFormat[] PROGMEM = {"temp %d %d"};
#define IN_SMS_TEMP_FORMAT 42
const char inSmsUnsub[] PROGMEM = {"unsub "};
#define IN_SMS_UNSUB 43
const char inSmsUnsubFormat[] PROGMEM = {"unsub %s"};
#define IN_SMS_UNSUB_FORMAT 44
const char levelAlertMsg[] PROGMEM = {"Level alert"};
#define LEVEL_ALERT_MSG 45
const char levelHighMsg[] PROGMEM = {"high"};
#define LEVEL_HIGH_MSG 46
const char levelLowMsg[] PROGMEM = {"LOW"};
#define LEVEL_LOW_MSG 47
const char lightAlertMsg[] PROGMEM = {"Light alert"};
#define LIGHT_ALERT_MSG 48
const char lightScheduleMsgFormat[] PROGMEM = {"Light schedule %s: %02d:%02d-%02d:%02d"};
#define LIGHT_SCHEDULE_MSG_FORMAT 49
const char lightScheduleSetMsg[] PROGMEM = {"Light schedule set (save ?)"};
#define LIGHT_SCHEDULE_SET_MSG 50
const char lightThresholdMsgFormat[] PROGMEM = {"Light limits %s: %d-%d"};
#define LIGHT_THRESHOLD_MSG_FORMAT 51
const char lightThresholdSetMsg[] PROGMEM = {"Light threshold set (save ?)"};
#define LIGHT_THRESHOLD_SET_MSG 52
const char measureAlertMsgFormat[] PROGMEM = {"ALERT %s - Temp: %.2f, Level: %s, Light: %d, POWER: %s"};
#define MEASURE_ALERT_MSG_FORMAT 53
const char measureMsgFormat[] PROGMEM = {"Temp: %.2f, Level: %s, Light: %d, POWER %s"};
#define MEASURE_MSG_FORMAT 54
const char newConfigMsg[] PROGMEM = {"NEW config"};
#define NEW_CONFIG_MSG 55
const char notConnectedGsmMsg[] PROGMEM = {"Check GSM PIN"};
#define NOT_CONNECTED_GSM_MSG 56
const char numberNotSubscribedMsg[] PROGMEM = {"Sub to %s failed"};
#define NUMBER_NOT_SUBSCRIBED_MSG 57
const char numberNotUnsubscribedMsg[] PROGMEM = {"Unsub to %s failed: not found"};
#define NUMBER_NOT_UNSUBSCRIBED_MSG 58
const char numberSubscribedMsg[] PROGMEM = {"Sub to %s done"};
#define NUMBER_SUBSCRIBED_MSG 59
const char numberUnsubscribedMsg[] PROGMEM = {"Unsub to %s done"};
#define NUMBER_UNSUBSCRIBED_MSG 60
const char powerAlertMsg[] PROGMEM = {"POWER Alert"};
#define POWER_ALERT_MSG 61
const char powerOffMsg[] PROGMEM = {"Off"};
#define POWER_OFF_MSG 62
const char powerOnMsg[] PROGMEM = {"On"};
#define POWER_ON_MSG 63
const char resetSubDoneMsg[] PROGMEM = {"Reset sub. done (save ?)"};
#define RESET_SUB_DONE_MSG 64
const char savingConfigMsg[] PROGMEM = {"Saving config"};
#define SAVING_CONFIG_MSG 65
const char sendingSmsMsg[] PROGMEM = {"Sending SMS..."};
#define SENDING_SMS_MSG 66
const char setAdminDoneMsg[] PROGMEM = {"Admin set (save ?)"};
#define SET_ADMIN_DONE_MSG 67
const char setTimeDoneMsg[] PROGMEM = {"Time set"};
#define SET_TIME_DONE_MSG 68
const char smsSentMsg[] PROGMEM = {"SMS sent"};
#define SMS_SENT_MSG 69
const char temperatureAdjustmentMsgFormat[] PROGMEM = {"Temp Adj: %d"};
#define TEMPERATURE_ADJUSTMENT_MSG_FORMAT 70
const char temperatureAdjustmentSetMsg[] PROGMEM = {"Temp adjustment set (save ?)"};
#define TEMPERATURE_ADJUSTMENT_SET_MSG 71
const char temperatureAlertMsg[] PROGMEM = {"Temp Alert"};
#define TEMPERATURE_ALERT_MSG 72
const char temperatureThresholdsSetMsg[] PROGMEM = {"Temp thresholds set (save ?)"};
#define TEMPERATURE_THRESHOLDS_SET_MSG 73
const char temperatureThresholdMsgFormat[] PROGMEM = {"Temp: %d %d"};
#define TEMPERATURE_THRESHOLD_MSG_FORMAT 74
const char tempInitMsg[] PROGMEM = {"DS1820 Test"};
#define TEMP_INIT_MSG 75
const char unknownMsg[] PROGMEM = {"Stop"};
#define UNKNOWN_MSG 76
const char unknownNumberMsg[] PROGMEM = {"UnknoWn number"};
#define UNKNOWN_NUMBER_MSG 77
const char* const messages[] PROGMEM = {
  accessDeniedMsg, addrErrMsg, alertMsgFormat, buildMsg, checkSmsMsg, 
  clearAlertDoneMsg, configResetMsg, configSavedMsg, connectedGsmMsg, connectingGsmMsg, 
  crcNotValidMsg, currentDateFormatMsg, discardSmsMsg, familyMsg, fromNumberMsg, 
  initAquamonMsg, initGsmMsg, intervalSetMsg, inSmsAbout, inSmsClearAlert, 
  inSmsConfig, inSmsInterval, inSmsIntervalFormat, inSmsLightSchedule, inSmsLightScheduleFormat, 
  inSmsLightThreshold, inSmsLightThresholdFormat, inSmsResetConfig, inSmsResetLcd, inSmsResetSub, 
  inSmsSave, inSmsSetAdmin, inSmsSetAdminFormat, inSmsSetTime, inSmsSetTimeFormat, 
  inSmsStatus, inSmsSub, inSmsSubs, inSmsSubFormat, inSmsTemp, 
  inSmsTempAdj, inSmsTempAdjFormat, inSmsTempFormat, inSmsUnsub, inSmsUnsubFormat, 
  levelAlertMsg, levelHighMsg, levelLowMsg, lightAlertMsg, lightScheduleMsgFormat, 
  lightScheduleSetMsg, lightThresholdMsgFormat, lightThresholdSetMsg, measureAlertMsgFormat, measureMsgFormat, 
  newConfigMsg, notConnectedGsmMsg, numberNotSubscribedMsg, numberNotUnsubscribedMsg, numberSubscribedMsg, 
  numberUnsubscribedMsg, powerAlertMsg, powerOffMsg, powerOnMsg, resetSubDoneMsg, 
  savingConfigMsg, sendingSmsMsg, setAdminDoneMsg, setTimeDoneMsg, smsSentMsg, 
  temperatureAdjustmentMsgFormat, temperatureAdjustmentSetMsg, temperatureAlertMsg, temperatureThresholdsSetMsg, temperatureThresholdMsgFormat, 
  tempInitMsg, unknownMsg, unknownNumberMsg
};