
#V1.0 2016/09
- Full config via SMS
- config saved in EEPROM
- monitors water level, light on level and schedule, light off level and schedule, temperature, main power
- send alerts via SMS
- logs info on serial line
- LCD display 2x20 with measures and messages
- battery backup
- extra phone number registration


#Planned evolutions

#V2.0
- Connects to home wifi network
- Logs all measures to external website

#V3.0
- Creates own private wifi network
- Monitors extra aquaMonitor devices connections. These ones won't have a GSM circuit, only wifi.
- Handles all connected devices to forward them configuration messages, and transmit their alerts by SMS.

#V4.0
- Any extra device will be able to create its own private wifi network to handle more devices
(wifi component only accepts 4 simultaneous connections. Could multiplex, but less fun, and not as quick)
