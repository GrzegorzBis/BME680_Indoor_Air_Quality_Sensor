# Introduction
This is a manual to an indoor air quality sensor based on BME680. Below, you can find the information on what do you need to build the device, how to build it and how to install the software.

# Components

**BME680** is a sensor that measures temperature, pressure, humidity and gas concentration. The specification is available in the datasheet posted here-[https://www.bosch-sensortec.com/bst/products/all_products/bme680](https://www.bosch-sensortec.com/bst/products/all_products/bme680) The product sold at nettigo.pl is an adapted version for the Wemos D1 Mini Pro. [https://nettigo.pl/products/modul-czujnika-bme680-dla-wemos-d1-mini](https://nettigo.pl/products/modul-czujnika-bme680-dla-wemos-d1-mini)

**Breadboard**- I used a regular breadboard with 63 rows of pins, however you only need at least 40. I wanted to avoid soldering, however it will be inevitable. Breadboard allows you to connect all the devices with cables. [https://nettigo.pl/products/plytka-stykowa-duza-830-otworow](https://nettigo.pl/products/plytka-stykowa-duza-830-otworow)

**Wemos D1 mini Pro** is a small computer. It has 16MB of flash memory and communicates with the sensor via I2C. It has a built-in Wi-Fi module. As a power supply, it uses a micro usb port.  [https://nettigo.pl/products/modul-wifi-wemos-d1-mini-pro](https://nettigo.pl/products/modul-wifi-wemos-d1-mini-pro)

**Screen**- I used a regular blue OLED screen with a resolution of 128 x 64 pixels. It is very simple and can be easily plugged. [https://nettigo.pl/products/wyswietlacz-oled-0-96-i2c-128x64-niebieski](https://nettigo.pl/products/wyswietlacz-oled-0-96-i2c-128x64-niebieski)

**Soldering kit**- In order to solder all the necessary elements with the goldpins, you will need:

-   Soldering iron > [https://nettigo.pl/products/stacja-lutownicza-kolbowa-yihua-936a-50-100w-200-480c](https://nettigo.pl/products/stacja-lutownicza-kolbowa-yihua-936a-50-100w-200-480c)
    
-   Solder- Tin and iron alloy > [https://nettigo.pl/products/cyna-0-70mm-100g-sn60pb40-sw26-cynel](https://nettigo.pl/products/cyna-0-70mm-100g-sn60pb40-sw26-cynel)
    
-   Rosin > [https://nettigo.pl/products/kalafonia-lutownicza-45g-cynel](https://nettigo.pl/products/kalafonia-lutownicza-45g-cynel)
    
-   Cleaner > [https://nettigo.pl/products/czyscik-do-grota-lutowniczego-z-podstawka](https://nettigo.pl/products/czyscik-do-grota-lutowniczego-z-podstawka) 

**10 cm Male-Male Cables** [https://nettigo.pl/products/przewody-m-m-10-cm-40-szt](https://nettigo.pl/products/przewody-m-m-10-cm-40-szt)

# Software

1. After connecting your board to a PC, you need to download neccessary files. In order to communicate, you need to install USB-UART bridge to communicate with your board. Wemos D1 mini Pro uses CP2104 and NodeMCU V2 uses CP2102. If you plan to use different boards to run my software, check what UART chip type it has and install a proper driver. You can find the drivers on this website. [https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers) After extracting all the files, open the selected file. Install it.
![](https://lh3.googleusercontent.com/dvlGf-V6EP6XEWwjdQu3R5E6zZ2UEuz_09Ohz3oLi8yGFVRDRYAGoXumeGcTqVzwJ6Qt9eRWDrQ)
![enter image description here](https://lh3.googleusercontent.com/m1vezmS26XTSscTT9mcFPhW-Sxt8R8LndQLp320v0q4s6PlngjCl9DOcU3qeDqW8ToCbLGPc1hI)    
2.  Find your device in the device manager.
![enter image description here](https://lh3.googleusercontent.com/YpVE5PCGoCcuIC7TRPuCLvEWWNPm49I_G-wG8be3wLCHfT4mt-XnkIIZm7q2gJ5iEPw4qOlhSa4)
    
3.  Install Arduino IDE- open [https://www.arduino.cc/en/Main/Software](https://www.arduino.cc/en/Main/Software) and find and download Arduino IDE.
![enter image description here](https://lh3.googleusercontent.com/-xmNJXDtYje-EGJMqDbXiBDEDbRD4y7sZopA2E214nyqbDp-guHKg7kj24xzuC8hTd4c_q5pbKM)
4.  Open Arduino IDE and choose file > preferences. Enter [http://arduino.esp8266.com/versions/2.5.0/package_esp8266com_index.json](http://arduino.esp8266.com/versions/2.5.0/package_esp8266com_index.json) into the Additional Board Manager URLs field.
 ![enter image description here](https://lh3.googleusercontent.com/CaLdrjtaWLLBrieb-AUochurwRJuJqcjLUZbWNX5K8pg_4TBV4ctuKPdlJW42UdiiQ11RRdXXp4)
Find Tools > Board  > Boards Manager and install "esp8266" platform. ![enter image description here](https://lh3.googleusercontent.com/QzdjbjXRzpC1ORajgbY0g9Vl30DD0omTyeAwhde40OfUazYQuawAbGt3ivmE3Q_0xvqoNaiUP4Y)
Don't forget to select your ESP8266 board from Tools > Board menu after installation. Set NodeMCU 1.0 (ESP-12E Module) for NodeMCU V2 or select LOLIN(WEMOS) D1 mini Pro for Wemos D1 mini Pro.
![enter image description here](https://lh3.googleusercontent.com/ylj90bBuOoaUbr3pzkkaBjApVJOnlQJpYI6MQ_vg0NE-MofZnAQLK-pXCObub1Q6VJm5MeKtwss)
6.  Change the Tools > Flash Size to 4M (3M SPIRFFS) for NodeMCU V2 and the Port to the one you found in device manager.
![enter image description here](https://lh3.googleusercontent.com/1P2Lb4rknNxG9vNqjf39zs5ofaE293vPA7zbJtiuwh66wnuZBpR1V5U84vaROf-vRq1MuC-oW4k)
    
7.  If you want to check whether you did everything properly, you can upload a sketch on your board. In order to do so, choose File > Examples > ESP8266 > Blink. In the new window, choose Sketch > Upload. The lamp on the board should start blinking.
    
8.  If everything worked correctly, you will now install the software.
9. Before you can install it, you need to get certain libraries. In order to find them you need to go to Sketch > Include Library > Manage Libraries

- Write "ssd1306" and choose the library created by Daniel Eichhorn and Fabrice Weinberg. I suggest the version 4.0.0

- Write "arduinoJson" and choose the one created by Benoit Blanchon. DO NOT INSTALL THE BETA VERSIONS. I strongly advise to install the version 5.13.4

- Write "BME680" and choose Adafruit BME680 Library by Adafruit. I use the 1.0.7 version.

- Write "Adafruit Sensor" and install Adafruit Unified Sensor. I use the version 1.0.2

- Write "thingspeak" and choose the one by MathWorks. I suggest the version 1.4.3
10. Congratulations! You successfully installed the software. If you want to be able to have access to the measurements online, you just have to set the Thingspeak platform. 
11. Create an account and login. Then, go to Channels > New Channel. Fill in the boxes![enter image description here](https://lh3.googleusercontent.com/ZkXxpW7bZCZfdU1JiwZJkwovNoAJE-Ld0HFGkM5IKv9T268GzvdqDA7e6uap-H6l2P1viHxREU0)
# Assembly
    

After obtaining all the components, we can start by soldering the goldpins to the Wemos D1 Mini Pro and the BME680 just like on the picture above. NodeMCU V2 is soldered by default. After soldering all the pins, plug Wemos/NodeMCU and the BME680 to the breadboard just like on the picture below. Plug the cables next to the pins and connect the components just like on the picture below.
