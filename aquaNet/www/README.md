
The content of this directory needs to be uploaded to a publicly accessible web server, in a subdirectory named "aquaNet" of the home directory.

The js and css files are loaded from the page served by the module to build the webApp, which allows to modify the webApp without the need for flashing the ESP8266 to improve the look, or translate messages.

The test.html page can be used to test the webApp, it will then use the getData.json test data.
It anticipates on V3 features, allowing to display data from several connected modules.

NB: the term "webApp" might be an overstatement.
While for now it just displays data, it will soon allow sending commands to set configuration stuff and more fun.

It currently looks like this:

<img src="../aquaNet.png" width="300px"/>
