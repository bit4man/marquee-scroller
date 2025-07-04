# Marquee Scroller (Clock, Weather, News, and More)

Based on the version from Qrome

## NOTICE
The latest version of Marquee Scroller 3.01 works with **ESP8266 Core 3.0.2** -- if you are upgrading from Marquee Scroller 2.X version this may require you to enter in all your API Keys and settings.  Always meake sure you have coppied all your API keys somewhere before updating.  The ESP8266 Core 3.0.2 uses the newer FS (file system) that may require a fresh start on the configuration.
Make sure you update to the latest version of WifiManager library (link below).
* Removed Bitcoin features in 3.0

## Important changes
* ArduinoJson upgraded to version 7
* PiHole v6 supported. Note important change in it's configuration
* SPIFFS replaced by LittleFS - note, you will need to redo your configuration as it resets your current config
* The PiHole "graphics" (chart) is reenabled and based on Pihole v6. This option will most likely be removed.

## Features include:
* Accurate Clock refresh off Internet Time Servers
* Local Weather and conditions (refreshed every 10 - 30 minutes)
* News Headlines from all the major sources
* Configured through Web Interface
* Display 3D print progress from your OctoPrint Server
* Option to display Pi-hole status and graph (each pixel accross is 10 minutes)
* Basic Authorization around Configuration web interface
* Support for OTA (loading firmware over WiFi)
* Update firmware through web interface
* Configurable scroll speed
* Configurable scrolling frequency
* Configurable number of LED panels
* Options of different types of Clock Displays (display seconds or temperature) -- only on 8 or more display panels
* Video: https://youtu.be/DsThufRpoiQ
* Build Video by Chris Riley: https://youtu.be/KqBiqJT9_lE

## Required Parts:
* Wemos D1 Mini: https://amzn.to/3tMl81U
* Dot Matrix Module: https://amzn.to/2HtnQlD  

Note: Using the links provided here help to support these types of projects. Thank you for the support.  

## Wiring for the Wemos D1 Mini to the Dot Matrix Display
CLK -> D5 (SCK)  
CS  -> D6  
DIN -> D7 (MOSI)  
VCC -> 5V+  
GND -> GND-  

![Marquee Scroller Wiring](/images/marquee_scroller_pins.png)  

## 3D Printed Case by David Payne:  
Original Single Panel version: https://www.thingiverse.com/thing:2867294  
Double Wide LED version: https://www.thingiverse.com/thing:2989552  

## Upgrading from version 2.5 or Higher
In version 2.6 and higher, the binary files that can be uploaded to your marque scrolling clock via the web interface.  From the main menu in the web interface select "Firmware Update" and follow the prompts.
* **marquee.ino.d1_mini_3.03.bin** - compiled for Wemos D1 Mini and standard 4x1 LED (default)
* **marquee.ino.d1_mini_wide_3.03.bin** - compiled for Wemos D1 Mini and double wide 8x1 LED display

## Compiling and Loading to Wemos D1
It is recommended to use Arduino IDE.  You will need to configure Arduino IDE to work with the Wemos board and USB port and installed the required USB drivers etc.  
* USB CH340G drivers:  https://sparks.gogo.co.nz/ch340.html
* Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.  This will add support for the Wemos D1 Mini to Arduino IDE.
* Open Boards Manager from Tools > Board menu and install esp8266 Core platform version Latest **3.0.2**
* Select Board:  "ESP8266 Boards (3.0.2)" --> "LOLIN(WEMOS) D1 R2 & mini"
* Set Flash Size: 4MB (FS:1MB OTA:~1019KB) -- **this project requires FS for saving and reading configuration settings.**
* Select the **Port** from the tools menu.  

## Loading Supporting Library Files in Arduino
Use the Arduino guide for details on how to installing and manage libraries https://www.arduino.cc/en/Guide/Libraries  
**Packages** -- the following packages and libraries are used (download and install):  
<WiFiManager.h> --> https://github.com/tzapu/WiFiManager (latest)  
<TimeLib.h> --> https://github.com/PaulStoffregen/Time  
<Adafruit_GFX.h> --> https://github.com/adafruit/Adafruit-GFX-Library  
<Max72xxPanel.h> --> https://github.com/markruys/arduino-Max72xxPanel  
<JsonStreamingParser.h> --> https://github.com/squix78/json-streaming-parser  

Note ArduinoJson (version 5.13.1) is now included as a library file in version 2.7 and later.

## Initial Configuration
Editing the **Settings.h** file is totally optional and not required.  All API Keys are now managed in the Web Interface. It is not required to edit the Settings.h file before loading and running the code.  
* Open Weather Map free API key: http://openweathermap.org/  -- this is used to get weather data and the latitude and longitude for the current time zone. Weather API key is required for correct time.
* TimeZoneDB free registration for API key: https://timezonedb.com/register -- this is used for setting the time and getting the correct time zone as well as managing time changes due to Day Light Savings time by regions.  This key is set and managed only through the web interface and added in version 2.10 of Marquee Scroller. TimeZoneDB key is required for correct time display.
* News API key (free): https://newsapi.org/ -- Optional if you want to get current news headlines.
* Your OctoPrint API Key -- optional if you use the OctoPrint status.
* Version 2.0 supports Chained 4x1 LED displays -- configure up to 16x1 in the Settings.h file.  
* Configure a "password" (API key) in Pihole which is required for pihole configuration                       

NOTE: The settings in the Settings.h are the default settings for the first loading. After loading you will manage changes to the settings via the Web Interface. If you want to change settings again in the settings.h, you will need to erase the file system on the Wemos or use the “Reset Settings” option in the Web Interface.

## PiHole v6 details
Given the limited memory of the Wemo d1, support for HTTPS cannot be added. This means an HTTP end point is required. Ensure this is not blocked if you intend to use PiHole.

PiHole v6 introduced a more modern API key feature, and changed the API formatting and features dramatically from prior versions. To connect to your PiHole installation, you must login to the Web Interface and under Settings -> "Web Interface / API" enable the Expert settings (top right) and click "Configure app password" under Advanced Settings. Copy the "new app password" on the popup screen, and click the "Replace App Password" or "Create App Password". This defines a single API key (the "password") that you need to use with Marque-scroller when setting up access.  Note, this does NOT change the password you use to login to the web-interface. Yes, it's confusing.  If you enter this feature and replace the password/key you must change it in the marque-scroller configuration too.

Note, this version of marque-scroller has not been tested with 2FA.

The "test the interface" button on the setting screen has not yet been configured for the POST method to extract a temporary SID. Save your settings, and you should see the web-page populate key data from pihole on the web-page. This means it's working.

Note, the concept of the status of PiHole has changed in v6. Instead of having a dedicated single value, v6 enumerates a status per DNS query type under the heading of 'status'.  To create a single value, the code was changed to /info/login's "dns" property, which indicates if PiHole is set to process DNS requests or not. If it is, a value of "Blocking" is used - if not "Stopped" is used. These values will show up on the marque scroller's PiHole section. Another value that's not mirgraed is "privacy_level" but since the value wasn't used, it remains blank.

The original version used historical "TimeData10mins" to filter out the number of blocked requests. A max of 144 blocks were collected and used to generate the "graphics" bar graph on the scroller. A graphics is generated as "bar graphs" for each entry using the maximum read value to indicate a full bar. Depending on how many matrix displays you have, only the numbers that fits a single display are shown. The rest is ignored. The current history view generates a record every 10 minutes which is used by this version. This means the code will collect up to a full day of historical blocking data, but only show a little more than 5 hours of data on a standard 4 matrix segment display.
All this means that the graphics is close to useless to understand how much is blocked. If every 10 minutes have just one more blocked request, the graph will show a full bar without showing any increase.  If you use the total number of requests as a max for a percentage, the bar will often just be 1 bar tall (1/8 = 12.5%). Meaning the value of the graph is very little regardless of methodology. For now the feature will remain.  The challenge and why the feature may be removed, is that the amount of data in the response does not fit a single string - the 8266 runs out of memory. This means a streaming trick is used instead, but you're still dealing with storing a lot of data for something that isn't providing the viewer anything productive.  Feel free to add issues/requests with ideas for a better visual to show PiHole activity.

## Web Interface
The Marquee Scroller uses the **WiFiManager** so when it can't find the last network it was connected to 
it will become a **AP Hotspot** -- connect to it with your phone and you can then enter your WiFi connection information.

After connected to your WiFi network it will display the IP addressed assigned to it and that can be 
used to open a browser to the Web Interface.  You will be able to manage your API Keys through the web interface.  
The default user / password for the configuration page is: admin / password  

The Clock will display the time of the City selected for the weather.  

<p align="center">
  <img src="/images/2018-04-19%2006.58.05.png" width="200"/>
  <img src="/images/2018-04-19%2006.58.15.png" width="200"/>
  <img src="/images/2018-04-19%2006.58.32.png" width="200"/>
  <img src="/images/2018-04-19%2006.58.58.png" width="200"/>
</p>

## Donation or Tip
Please do not feel obligated, but donations and tips are warmly welcomed.  I have added the donation button at the request of a few people that wanted to contribute and show appreciation.  Thank you, and enjoy the application and project.  

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=A82AT6FLN2MPY)

Or -- you can buy me something from my Amazon Wishlist: https://www.amazon.com/hz/wishlist/ls/GINC2PHRNEY3  

## Contributors
David Payne  
Nathan Glaus  
Daniel Eichhorn -- Author of the TimeClient class (in older versions)  
yanvigdev  
nashiko-s  
magnum129  
bit4man

Contributing to this software is warmly welcomed. You can do this basically by forking from master, committing modifications and then making a pulling requests against the latest DEV branch to be reviewed (follow the links above for operating guide). Detailed comments are encouraged. Adding change log and your contact into file header is encouraged. Thanks for your contribution.

When considering making a code contribution, please keep in mind the following goals for the project:
* User should not be required to edit the Settings.h file to compile and run.  This means the feature should be simple enough to manage through the web interface.
* Changes should always support the recommended hardware (links above).

![Marquee Scroller](/images/5d7f02ccbf01125cabbf246f97f2ead1_preview_featured.jpg)  
![Marquee Parts](/images/1ffa0c835554d280258c13be5513c4fe_preview_featured.jpg)
