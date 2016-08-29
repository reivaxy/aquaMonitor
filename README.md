# aquaMonitor
Monitoring a recifal aquarium with an Arduino Mega board, and sending alerts by SMS

Monitors light, temperature, and water level, and sends an SMS to a registered mobile phone number when configurable thresholds are reached.

All configuration handled through SMS.

Project comes with 3D files to print the triple sensor bracket (light, temperature and adjustable water level sensor) and case, and PCB design for Fritzing.

Disclaimer: This system description is provided with no warranty whatsoever. If you use it, you accept to do so at your own risks, should a bug or any event be the cause of a malfunction.

# Available features
* Temperature periodical measurement
* Light periodical measurement
* Water level (high/low) detection
* LCD display of these measures
* Sending SMS with light, temperature and level when thresholds are reached
* Can subscribe a new phone number to alerts by sending an SMS.
* Adjusting the low light alarm threshold by SMS
* Adjusting the temperature value by SMS
* Adjusting the time interval when light should be on.
* Save and read params (thresholds...) to EEPROM
* Battery in case main power fails.
* Most strings stored in PROGMEM to save variable memory space.
* Subscribed phone numbers can set the minimum interval between 2 alerts to avoid flooding
* Periodical checking for incoming SMS, handles messages like :
  * 'admin +33xxxxxxxx' : add this phone number with admin rights
  * 'config' : send back the config
  * 'display': Reset the display when messy (prototype contact issue ?, investigating)
  * 'interval xxx' : No more than 1 sms per xxx seconds
  * 'light XXX' : set the low light threshold for alerts
  * 'save': save the configuration (light schedule and level, temp range, subs, admin, ...) to EEPROM
  * 'schedule hh:ss - hh:ss' : set the time span during which light should be above threshold
  * 'status': send SMS back with current measures
  * 'sub reset': resets all subscriptions (admin only)
  * 'sub xxx' : subscribe to a service (alert, event)
  * 'subs' : send back list of subscribed phone numbers (admin only)
  * 'temp XXX YYY': set the low (XXX) and high (YYY) thresholds for temperature
  * 'temp adj XXX': set the temperature adjustment signed value
  * 'time 2016/07/30 11:00' : set the date and time
  * 'unsub xxx' : unsubscribe to a service (alert, event)

  
# TODOs
* Water movement detection to warn about pumps failure (sensor remains to be found...) 
* Some more configuration by SMS (level detection switch state inversion for instance, or display shift frequency ?)
* 'clear' sms to cancel the interval before next alert can be sent
* 'reset' sms to reset the unsaved configuration changes
* Periodical status sending (upon registration) ?


# Some pictures 

Prototype:<br/>
<img src="http://www.adgjm.eu/img/github/aquaMonitor-1024.jpg" width="300px"/><br/>

<br/><br/>
Finished:<br/>
<img src="http://adgjm.eu/img/github/finished.jpg" width="400px"/>

<br/><br/>
Installed:<br/>
<img src="http://adgjm.eu/img/github/installed.jpg" width="400px"/>

<br/><br/>
Sensors:<br/>
<img src="http://adgjm.eu/img/github/sensors.jpg" width="400px"/>

<br/><br/>
SMS exemples:<br/>
SMS sent by user are yellow, replies sent by device are blue.<br/>
<img src="http://adgjm.eu/img/github/sms.png" width="300px"/>

#License

<a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/">Creative Commons Attribution-NonCommercial 4.0 International License</a>.
