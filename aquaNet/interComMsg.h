
// This file contains messages exchanged between
// the arduino and the ESP8266
// They can also be type on the ESP serial line or in the
// appropriate input field on the ESP server web page
// The ESP send a message with the '#' prefix so the arduino
// knows it should interpret it and provide an answer with the
// same keyword (but no '#').
// Message sent with no prefix are copied to the arduino USB Serial
// Messages sent with the '@' prefix are arduino commands (the same
//   it can receive by SMS or by its usb serial line).

#define REQUEST_IF_GSM "chkGSM"  // esp checks if module is equipped with GSM
#define REQUEST_MEASURES "status"  // esp asks arduino for all measures


#define REQUEST_TIME "time" // esp asks arduino for the current time