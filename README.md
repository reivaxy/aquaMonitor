# aquaMonitor
Monitoring a recifal aquarium with an Arduino Mega board, and sending alerts by SMS

Monitors light, temperature, and water level, and sends an SMS to a registered mobile phone number when configurable thresholds are reached.

All configuration handled through SMS.

Project comes with 3D files to print the triple sensor bracket (light, temperature and adjustable water level sensor) and case, and PCB design for Fritzing.


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
  * 'status': send SMS back with current measures
  * 'sub xxx' : subscribe to a service (alert, event)
  * 'unsub xxx' : unsubscribe to a service (alert, event)
  * 'interval xxx' : No more than 1 sms per xxx seconds
  * 'light XXX' : set the low light threshold for alerts
  * 'schedule hh:ss - hh:ss' : set the time span during which light should be above threshold
  * 'temp XXX YYY': set the low (XXX) and high (YYY) thresholds for temperature
  * 'temp adg XXX': set the temperature adjustment signed value
  * 'sub reset': resets all subscriptions (admin only)
  * 'config' : send back the config
  * 'subs' : send back list of subscribed phone numbers (admin only)
  * 'save': save the configuration to EEPROM
  * 'display': Reset the display when messy (contact issue, investigating)

# TODOs
* Setting or adjusting the clock date and time (approx...) by SMS
* water movement detection to warn about pumps failure (sensor remains to be found...) 
* Some more configuration by SMS (level detection switch state inversion for instance)
* Send an SMS in case of main power failure.


# Some pictures 

<img src="http://www.adgjm.eu/img/github/aquaMonitor-1024.jpg" width="300px"/><br/>
Prototype.
<br/><br/>
<img src="http://www.adgjm.eu/img/github/display.jpg" width="300px"/><br/>
LCD display test, no sensor attached, no GSM module.


#License

<a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/">Creative Commons Attribution-NonCommercial 4.0 International License</a>.
