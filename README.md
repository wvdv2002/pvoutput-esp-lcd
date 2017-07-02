# PVOUTPUT ESP8266 LCD.

Using this repository and an ILI9341 320*240 LCD you can create your own [Pvoutput](www.pvoutput.org) monitor screen. This great website is used to log data coming from green energy systems. This screen can then be put in your livingroom to keep a close eye on how much green energy you are generating.

#Usage
NOTE: The code may have a few rough edges and no comments at all. You are free to ask any questions though.
This piece of software was coded together with [Bob van der Linden](https://github.com/bobvanderlinden)

Make sure you have installed the following Arduino Libraries:
- [Adafruit-GFX-Library]
- [Adafruit-ILI9341] library
- [WifiManager]
- [SPIFFS]
- [ESP8266 Sketch data upload](https://github.com/esp8266/arduino-esp8266fs-plugin)

When these are installed, compile and upload the code to an ESP8266 connected via SPI to a ILI9341 LCD screen. 

Pinout:
ILI9341 pin	ESP8266 pin
MISO 		GPIO12
MOSI 		GPIO13
SCK 		GPIO14
CS 		GPIO15
DC 		GPIO2
Backlight Brightness GPIO0

When the esp8266 is flashed and the sketch data is uploaded to the SPIFFS, follow the instructions on the LCD to connect to the ESP8266. The first time the used WifiManager library will setup an Acces point, connect to it and use the setup to set up the Wifi network to connect to. After the connection is established. Setup and view the needed parameters, by going to the corresponding link:

*IPAddess*/apikey?apikey=*PVOUTPUTREADAPIKEY* PVoutput account api key(can be found in the user settings of your pvoutput account)
*IPAddess*/refresh //Refresh all data
*IPAddess*/systemid //Set the systemId which to view.
*IPAddess*/wifisetup //Redo wifi setup
*IPAddess*/reboot //Reboot the ESP8266
*IPAddess*/brightness?brightness=*Number between 0-1023*

When all is setup, you should see a screen with your energy source information on it, a graph from the last hours and some totals.
