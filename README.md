# PremierPass

Pass Training system.

## Getting Started

This code was tested with an ATMega328P, ESP8266 Thing, and Adafruit MicroSD Breakout Board++.

### Prerequisites

This project was made for the Arduino IDE and Xbee.
To program the Thing you'll need to download the ESP8266 Add-On for Arduino.
https://github.com/esp8266/Arduino
The Thing runs the webapp from flash memory using the SPIFFS filesystem, so you'll need to set that up as well:
http://esp8266.github.io/Arduino/versions/2.0.0/doc/filesystem.html
And download the uploading tool for SPIFFS:
https://github.com/esp8266/arduino-esp8266fs-plugin
You'll also need to use a workaround to use both SPIFFS and the SD library simultaneously. It changes references to the SD File type to SDFile, see:
https://www.esp8266.com/viewtopic.php?f=32&t=12023


### Installing

Make sure Thing code is on a file in the Arduino, with data file inside it.
Open wifi.ino in Arduino IDE.
Verify the sketch compiles.
Upload data to SPIFFS using the tool.
Upload the sketch.

Upload ATMega328p code.

## Testing

Connect to the access point:
ssid: "PremierPass"
password: "premierpass"

Head to premierpass.local/

### Understanding the code

Each section or function is commented for readability, please see for more information

### XBee Information

This configuration profile was made for and used on XBee S1 chips. Configuration is done through the XCTU program made by Digi. It utilizes the XBee Digimeshprotocol to communicate between each other when connected to Arduinos via serial. This works with the included Arduino code.Loading these configuration profile on any 2 Xbees will not allow the for communication between them with the included app. The functions "sendData" and "aggregateData" are specific to the receiving Xbees 64-bit address, aka serial number, which cannot be changed. Therefore, these functions must be modified with the proper serial number AND CHECKSUM in order for them to work.


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc
