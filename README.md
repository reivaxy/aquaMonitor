# aquaMonitor
Monitoring a recifal aquarium with arduino, and sending alerts by SMS

Monitors light and temperature, and sends an SMS to a registered mobile phone number when configurable thresholds are reached.

Thresholds (and other stuff) can be configured with an (optional) IR remote control, and by sending an SMS to the arduino


When complete, will probably not fit on Arduino UNO but will require a Mega

# Available features
* Temperature periodical measurement
* Light periodical measurement
* Water level detection
* LCD display of these measures
* Sending SMS with light and temperature when thresholds are reached
* Adjusting the low light alarm threshold by IR remote control & by SMS
* Adjusting the temperature value by IR remote control & by SMS
* Save and read params (thresholds...) to EEPROM
* Most strings stored in PROGMEM to save variable space.
* Periodical checking for incoming SMS, handles messages like :
  * 'status': send SMS back with current measures
  * 'sub xxx' : subscribe to a service (alert, event)
  * 'unsub xxx' : unsubscribe to a service (alert, event)
  * 'interval xxx' : No more than 1 sms per xxx seconds
  * 'light XXX' : set the light threshold for alerts
  * 'schedule hh:ss - hh:ss' : set the time span during which light should be above threshold
  * 'temp XXX YYY': set the low (XXX) and high (YYY) thresholds for temperature
  * 'temp adg XXX': set the temperature adjustment signed value
  * 'sub reset': resets all subscriptions (admin only)
  * 'config' : send back the config
  * 'subs' : send back list of subscribed phone numbers, for admin only
  * 'save': save the configuration to EEPROM
* Subscribed phone numbers can set the minimum interval between 2 alerts to avoid flooding


# TODOs
* Setting or adjusting the clock date and time (approx...) by SMS
* water movement detection to warn about pumps failure (sensor remains to be found...) 
* backuped power (and alert) in case of power failure
* Saving periodical measure data on optional SD card (and may be info about sent and received SMS)
* Some more configuration by SMS (level detection switch state inversion for instance)

#Licence
...to kill.

Feel free to use and modify and let me know.
