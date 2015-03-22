# aquaMonitor
Monitoring a recifal aquarium with arduino, and sending alerts by SMS

For now, more POC than anything.

Monitors light and temperature and sends an SMS to a pre defined mobile phone number when configurable thresholds are reached

Thresholds can be configured through an IR remote control

When complete, will probably not fit on Arduino UNO.

# TODOs
* RTC to handle lighting time schedule and not send SMS just because lights are off when they should be
* water level detection to warn about evaporation compensation failure
* water movement detection to warn about pumps failure (sensor remains to be found...) 
* centralized LCD message display to insure transient messages display and removal
* centralized SMS system to not send continuously while issue is not fixed
* backuped power (and alert) in case of power failure

#Licence
...to kill.

Feel free to use and modify and let me know.
