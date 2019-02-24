# Introduction
An indoor air quality sensor based on BME680 measures temperature, humidity, barometric pressure and gas resistance. Below, you can find the information need to build the device including control software. Control software for the sensor is based on open software from www.luftdaten.info Outdoor Air Quality sensor project.
 ![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/BME680_Indoor_Air_Quality_Sensor.jpg)

# Components

- **[BME680 board](https://nettigo.pl/products/modul-czujnika-bme680-dla-wemos-d1-mini)** is a sensor that measures temperature, pressure, humidity and gas concentration. More details on the sensor can be found in the datasheet, which is available here: [BME680 datasheet](https://www.bosch-sensortec.com/bst/products/all_products/bme680) There are many variants of BME680 boards available, in my project I am using BME680 from nettigo: [BME680 Wemos board](https://nettigo.pl/products/modul-czujnika-bme680-dla-wemos-d1-mini)

- **[Breadboard](https://nettigo.pl/products/plytka-stykowa-duza-830-otworow)**- I used a regular breadboard with 63 rows of pins, however you only need 35. I wanted to avoid soldering, however it will be inevitable. Breadboard allows you to connect all the devices with cables.

- **[Wemos D1 mini pro board](https://nettigo.pl/products/modul-wifi-wemos-d1-mini-pro)** is a small single board computer. It has 16MB of flash memory and I2C bus to communicate with external sensors. It has a built-in Wi-Fi module. For host access and power it uses a micro usb port.

- **[OLED Screen](https://nettigo.pl/products/wyswietlacz-oled-0-96-i2c-128x64-niebieski)**- I used a regular blue OLED screen with a resolution of 128 x 64 pixels. It is very simple and can be easily plugged.

- **[10cm Male-Male Cables](https://nettigo.pl/products/przewody-m-m-10-cm-40-szt)**

- **[Goldpins](https://nettigo.pl/products/goldpin-zlacze-wtyk-prosty-1x40-raster-2-54-mm)**

- **Soldering kit**- In order to solder all the necessary elements with the goldpins, you will need:
     - [Soldering iron](https://nettigo.pl/products/stacja-lutownicza-kolbowa-yihua-936a-50-100w-200-480c)

     - [Solder - tin and iron alloy](https://nettigo.pl/products/cyna-0-70mm-100g-sn60pb40-sw26-cynel)
    
     - [Rosin](https://nettigo.pl/products/kalafonia-lutownicza-45g-cynel)
    
     - [Cleaner](https://nettigo.pl/products/czyscik-do-grota-lutowniczego-z-podstawka) 
# Software

After connecting your board to a PC using USB cable, you need to download set of neccessary files.
1. In order to communicate with the board, you need to install USB-UART bridge SW. Depending on the CPU board you are going to use, different USB-UART bridge driver might be needed. As Wemos D1 mini Pro uses CP2104 USB-UART bridge and NodeMCU V2 uses CP2102 USB-UART bridge you will have to install for them CP210X SW driver. If you plan to use different boards to run my software, check what UART chip type it has and install a proper driver. You can dowload CP210X driver from this website: [SiLabs CP210X USB-UART bridge driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers). After extracting all the files from the archive, for 64-bit system install x64 installer, for 32-bit system install the x86 one.
![](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/SCR2.jpg)
2.  Find COM port assigned to your device. To do so, open the Windows Device Manager and look for Ports (COM&LPT) -> Silicon Labs CP210X USB to UART Bridge.
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/SCR5.jpg)

3.  Install Arduino IDE - open [Arduino Project Web Page](https://www.arduino.cc/en/Main/Software),find and download Arduino IDE.
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/SCR6.jpg)
4.  Install ESP8266 Board Support package. Open Arduino IDE and select from menu: File -> Preferences. Enter [http://arduino.esp8266.com/versions/2.5.0/package_esp8266com_index.json](http://arduino.esp8266.com/versions/2.5.0/package_esp8266com_index.json) into the Additional Board Manager URLs field.
 ![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/SCR12.jpg)
Select Tools->Board->Boards Manager and install "esp8266" platform.
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/SCR14.jpg)
Don't forget to select proper board type after ESP8266 package installation. In Tools->Board menu set NodeMCU 1.0 (ESP-12E Module) for NodeMCU V2 or LOLIN(WEMOS) D1 mini Pro for Wemos D1 mini Pro.
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/SCR15.jpg)
6.  Change the Tools->Flash Size to 4M (3M SPIRFFS) for NodeMCU V2 and the Port to the one you found in device manager(step2 above).
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/SCR16.jpg)
    
7.  To check whether you did everything properly, you can upload a test sketch on your board. In order to do so, select File->Examples->ESP8266->Blink. To build and upload program to the device, in the new window, select Sketch->Upload. The lamp on the board should start blinking, after the program will be loaded.
    
8.  If everything worked correctly, you will now proceed to installing the control software for sensor.
9.  Before you can install it, you need to install set of Arduino libraries. Go to Sketch->Include Library->Manage Libraries, install the following libraries:
- ESP 8266 and ESP32 Oled Driver for SSD1306 by Daniel Eichhorn and Fabrice Weinberg, version 4.0.0
- ArduinoJson by Benoit Blanchon, version 5.13.4. DO NOT INSTALL THE BETA VERSIONS.
- Adafruit BME680 Library by Adafruit, version 1.0.7
- Adafruit Unified Sensor by Adafruit, version 1.0.2
- ThingSpeak by MathWorks version 1.4.3
10. Clone or download a zip file with the source code for the BME680 Indoor Air Quality Sensor from https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor
10. Congratulations! You successfully installed the software. The next installation is optional. If you want to be able to have access to your measurements online, or you want to track history of your measurements you have to set the Thingspeak support. 
11. Create an account and login. Then, go to Channels > New Channel. Fill in the boxes![enter image description here](https://lh3.googleusercontent.com/ZkXxpW7bZCZfdU1JiwZJkwovNoAJE-Ld0HFGkM5IKv9T268GzvdqDA7e6uap-H6l2P1viHxREU0)
# Assembly
    
After obtaining all the components, we can start by soldering the goldpins to the Wemos D1 Mini Pro and the BME680 just like on the picture below.

![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/BME680_WITH_GOLDPINS.jpg)

NodeMCU V2 has goldpins soldered by default. In the next step plug Wemos/NodeMCU, the BME680 and OLED screen boards to the breadboard just like on the picture at the top of this page. Plug the cables using schematics shown below.
**Connection scheme for BME680 Indoor Air Quality Sensor with Wemos**
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/Wemos_BME680_OLED.jpg)

For Wemos I2C bus where BME680 and OLED screen will be connected has the following layout:
- D1 pin -> I2C SCL
- D2 pin -> I2C SDA

**Connection scheme for BME680 Indoor Air Quality Sensor with NodeMCU**
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/NodeMCU_BME680_OLED.jpg)

For NodeMCU I2C bus where BME680 and OLED screen will be connected has the following layout:
- D3 pin -> I2C SDA
- D4 pin -> I2C SCL

Both BME680 board and OLED screen are powered from 3.3V pin.

# IAQ Sensor User Guide

After the IAQ sensor software is installed on the sensor, please login to the IAQsensor-ID WIFI network (where ID is the unique number assigned to the sensor board). The following screen will appear after login in:
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/Login_1.jpg)
Please select one of the available WiFi networks and provide password for it. In the next step IAQ sensor will automatically log in into the selected WiFi network. New IP address assigned automatically to the IAQ sensor will be displayed on the IAQ sensor screen.
When the sensor is loged in into the WiFi network, further access to the IAQ sensor can be done using Web browser, eg. google-chrome or mozilla-firefox and the IP address displayed on the sensor screen. On the picture below the IAQ sensor home page is presented.
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/homepage.jpg)
In order to perform sensor configuration, please select the configuration button on the sensor home page. On the picture below the IAQ sensor configuration page is presented.
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/configuration.jpg)
In order to get access to the current IAQ sensor readings, please select the current data button on the sensor home page. On the picture below the IAQ sensor current data page is presented.
![enter image description here](https://github.com/GrzegorzBis/BME680_Indoor_Air_Quality_Sensor/blob/master/pictures/currentdata.jpg)
