# aquaNet

This is the optional Wifi device that will allow you to:
* Access aquaMonitor through a web interface to modify the settings and run commands
* Have aquaMonitor periodically log measures to an external web site in order to build statistics
* Have several aquaMonitor modules share one GSM (hence one SIM card) to send alerts.

Use the Arduino IDE to build and upload the binary to your ESP8266 module.
The steps to set up the IDE are described here: http://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/

The embedded web server actually serves a minimalistic page that load scripts from your an external web site you own, that builds the webApp.
It makes styling the page very easy and with no need to flash the wifi module again.

It uses jQuery, Backbone and Boostrap.

Here is what the webApp currently looks like, showing data for one module, with an alert on the light level while editing a light setting.

<img src="aquaNet.png" width="300px"/>
