# PremierPass
#Pass Training system.

##Executive Summary
###The Problem

The purpose of this project is to design the structure of a passing training system for the Men’s and Women’s Northwestern Soccer Teams. We began with a system using rigid gates and bright, centrally located light indicators designed by a prior team in DSGN 384. Our goal is to create a system that can reliably sense balls entering a goal, communicate the data to a central location, and inform users of their performance on an interactive and easy to use platform.  

###Our Solution  

The PremierPass System consists of four metal goals that can be arrayed in any way by the user to best help them practice their passing. The goals feature LED lights to indicate which goal should be passed to, nets to contain the balls, a staking system to keep the goals in place during the drill, and a transportation system to help the user transport the drill parts. The system is equipped with infrared (IR) sensors to detect successful passes. PremierPass records data about the performance of individual players and displays it on a web app.  

###How it works  

The rigidly connected metal goal protects the electronics and allows it to remain in place with repeated ball impacts. The LED lighting is bright and conspicuous enough to be used both indoors and outdoors and in many different lighting conditions. IR sensors are used to detect whether balls pass through the plane of the goal. This data is aggregated and communicated wirelessly to a web app displayed on a user’s personal device. Netting behind the goal contains the balls during the drill without interfering with the sensors and while maintaining ease of transportation. The 3D-printed stakes also help to keep the system in place upon soccer ball impacts to present reset of the drill. The transportation system utilizes a cart that can contain all the goals, a ball bag, battery chargers, and the measuring tape used for setup.  


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
You'll also need to use a workaround to use both SPIFFS and the SD library simultaneously. It changes references to the SD.  
File type to SDFile, see:   
https://www.esp8266.com/viewtopic.php?f=32&t=12023  


### Installing

Make sure Thing code is on a file in the Arduino, with the data file inside it.  
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
