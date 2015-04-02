# aquaMonitor
Monitoring a recifal aquarium with arduino, and sending alerts by SMS

Monitors light and temperature, and sends an SMS to a pre defined mobile phone number when configurable thresholds are reached.

The WihoutIR branch is a test to check how much emory is saved by removing IR support: About 7k
That should be enough to start working on RTC integration + schedule.
All settings will be made available through incoming SMS processing
<del>Thresholds (and other stuff) can be configured with an IR remote control.</del>

Can also respond to SMS sent to it, according to their contents.


When complete, will probably not fit on Arduino UNO.

# Available features
* Temperature periodical measurement
* Light periodical measurement
* LCD display of these measures
* Sending SMS with light and temperature when thresholds are reached
* <del>Adjusting the low light alarm threshold by IR remote control & LCD display</del>
* <del>Adjusting the temperature value by IR remote control & LCD display</del>
* <del>Displaying the (hardcoded) low and high temperature alarm thresholds by IR remote control & LCD display</del>
* <del>Displaying the (hardocded) SMS recipient phone number by IR remote control & LCD display</del>
* Limitation on how often alarm SMS can be sent (still hardcoded)
* Periodical checking for incoming SMS
  * 'status': send SMS back with current measures
  * 'sub xxx' : subscribe to a service (alert, event)
  * 'unsub xxx' : unsubscribe to a service (alert, event)
* Save and read params (thresholds...) to EEPROM
* Most strings stored in PROGMEM to save variable space. But a Mega will be necessary...

# TODOs
* handle incoming SMS for all seetings
* smart configuration saving to EEPROM (not each time, within a delay after a modification, on request per SMS...)
* RTC with onboard battery to handle lighting time schedule and not send SMS just because lights are off when they should be
* water level detection to warn about evaporation compensation failure
* water movement detection to warn about pumps failure (sensor remains to be found...) 
* centralized LCD message display to insure transient messages display and removal
* backuped power (and alerting) in case of power failure
* Loging: save periodical measure data to SD card (and may be also info about sent and received SMS)


#Licence
...to kill.

Feel free to use and modify and let me know.
