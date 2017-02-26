
#Statistics web server and webApp files host

This server provides an access point to record tank measures (from different tanks and users) into a database, as well as a page to display a graph from the data for a given tank:

<img src="https://github.com/reivaxy/aquaMonitor/blob/master/res/chart.png" width="300px"/>


The content of the www directory needs to be uploaded to a publicly accessible web server.

This web server hostname needs to be set into DEFAULT_WEBAPP_HOST constant in aquaMonitorSecret.h file (file that needs to be accessible when compiling the arduino and the ESP8266 firmwares, obviously not included in this repository).

The credentials to connect to your mySQL DB need to be set in the includes/myconfig.inc.php file, that must not be accessible through your server url.

The reeftanklpaqua.sql file should be used to create the DB structure (run it from command line or phpMySqlAdmin)

(Beware, there is no authentication yet between the arduino wifi module and the server, so anyone knowing the endpoints can inject data)


The www/aquaNet subdirectory contains files for the aquaNet webApp, which is a web application exposed by the wifi module.

The js and css files are loaded from the minimalist loader page served by the module to build the webApp, which allows to modify the webApp without the need for flashing the ESP8266 to improve the look, or translate messages.
This also makes using images really easier, and contributes to faster page loading, since the wifi module does not have a great bandwidth (and does not process multiple requests simultaneously).

Of course this is made possible thanks to the .htaccess directive:

Header set Access-Control-Allow-Origin "*"

The test.html page can be used to test the webApp, it will then use the getData.json test data.
It anticipates on V3 features, allowing to display data from several connected modules.

All translatable labels used in the webApp are in a css file, that will make localisation easy.



