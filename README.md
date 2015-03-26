# aquaMonitor
Monitoring a recifal aquarium with arduino, and sending alerts by SMS

For now, more POC than anything.

Monitors light and temperature and sends an SMS to a pre defined mobile phone number when configurable thresholds are reached

Thresholds can be configured through an IR remote control

When complete, will probably not fit on Arduino UNO.

# Available features
* Temperature periodical measurement
* Light periodical measurement
* LCD display of these measures
* Sending SMS with light and temperature when thresholds are reached
* Adjusting the low light alarm threshold by IR remote control & LCD display
* Adjusting the temperature value by IR remote control & LCD display
* Displaying the (hardcoded) low and high temperature alarm thresholds by IR remote control & LCD display
* Displaying the (hardocded) SMS recipient phone number by IR remote control & LCD display
* Limitation on how often alarm SMS can be sent (still hardcoded)
* Periodical checking for incoming SMS
  * If SMS is 'status', send SMS back with current measures


# TODOs
* RTC with onboard battery to handle lighting time schedule and not send SMS just because lights are off when they should be
* water level detection to warn about evaporation compensation failure
* water movement detection to warn about pumps failure (sensor remains to be found...) 
* centralized LCD message display to insure transient messages display and removal
* backuped power (and alert) in case of power failure
* Save and read params (thresholds...) to EEPROM


#Licence
...to kill.

Feel free to use and modify and let me know.
