# ESP with AT commands
In order to use ESP8266-01 wifi module with AT, some steps need to be processed as follows:
1. In the Arduino IDE, click File -> Preferences -> add the line 
http://arduino.esp8266.com/stable/package_esp8266com_index.json as in Figure 1.
![Board manager](https://raw.githubusercontent.com/minhminhng/pistonhand/esp8266_mqtt/BoardManager.png "Figure 1")
Figure 1. Adding board manager url. 

2. Sketch -> Include library -> search for ESP8266 library and install it.
After that, the Generic ESP8266 Module board will be included in the list of boards 
as well as the example sketches will appear when the board is selected.

Since the available boards only include two GPIO pins, it would be difficult to 
send data from multiple types of sensors, it should be used in cooperation with 
an Arduino board which possesses more input/output pins. They are connected through 
wires and the Arduino board will send command to the wifi module using AT commands. 
Those commands are available after the AT firmware is flashed to the module. 
Several flashing tools are obtainable from open sources as follows:

* [ESPflash tool](https://www.espressif.com/en/support/download/other-tools) for Windows user with an easy-to-use interface.
* Arduino tools located at $HOME/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool
* Esptool: https://github.com/espressif/esptool

In this project, we used the two available resources including Arduino IDE esptool and esptool from espressif. The latter tool can show more information relating to the chip such as chip type, MAC address. Additionally, it could flash the firmware (less than 30 seconds) faster than the esptool from Arduino IDE (proximately one minute) as observed from the flashing process.

Two versions of AT firmwares were flashed as follows:
1. Older version v0.9.5AT firmware 
  * Arduino tools with terminal: 
```
$HOME/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd ck -cb 115200 -cp 
/dev/ttyUSB0 -ca 0x00000 -cf /path/to/the/AT/firmware/v0.9.5.2ATFirmware.bin
```
  * Using esptool from espressif: 
```
esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 0x00000 
/path/to/the/AT/firmware/v0.9.5.2ATFirmware.bin
```
2. [Newest version](https://www.espressif.com/en/products/hardware/esp-wroom-02/resources) 
of ESP8266 AT firmware 
  * Arduino tools in terminal (for both ESP modules): 
```
$HOME/.arduino15/packages/esp8266/tools/esptool/0.4.13/esptool -vv -cd ck -cb 115200 -cp /dev/ttyUSB0 
-ca 0x00000 -cf /path/to/the/AT/firmware/boot_v1.2.bin 
-ca 0x01000 -cf /path/to/the/AT/firmware/user1.1024.new.2.bin 
-ca 0x7c000 -cf /path/to/the/AT/firmware/esp_init_data_default_v05.bin 
-ca 0x7e000 -cf /path/to/the/AT/firmware/blank.bin
``` 
  * Using esptool from espressif (only worked for the 512 kb module): 
```
esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash 
0x00000 /path/to/the/AT/firmware/boot_v1.2.bin 
0x01000 /path/to/the/AT/firmware/user1.1024.new.2.bin 
0x7c000 /path/to/the/AT/firmware/esp_init_data_default_v05.bin 
0x7e000 /path/to/the/AT/firmware/blank.bin
``` 

The memory allocation was achieved from the README.md file going along with the 
binary files in the above link.

Two ESP8266 wifi modules were used to test the connections including (1) ESP8266 302014 
with 512 kb flash size and (2) ESP8266 302015 AI cloud Inside with 1Mb flash size. 
The flash size was determined with the esptool developed by espressif. The two modules 
possess some different in their properties. Module (1) can be flashed with both tools 
and both firmware. Module (2) has taken some uploading efforts but could only succeed
 when flashing the newer firmware version with esptool from espressif. This is not of 
 our interest, so we did not progress it further.


In order to run AT commands, the flash pins must be disconnected. In order to join 
a network, the work mode of the ESP needs to be set to STA or STA+AP 
(AT+CWMODE=1 or AT+CWMODE=3). Other AT commands can be found from 
[internet](https://www.electrodragon.com/w/ESP8266_AT-Command_firmware) and 
[official document](https://www.espressif.com/en/support/download/documents) of 
the manufacturer. They must be used without space otherwise the terminal will 
raise “ERROR”.

This functionality allows the wifi module transferring the information to a webpage. 
In this project, we tested it with Thingspeak and sent information from an Arduino
 board to the wifi module through Serial communication.

[ESPflash tool]: https://www.espressif.com/en/support/download/other-tools
[Newest version]: https://www.espressif.com/en/products/hardware/esp-wroom-02/resources
[internet]: https://www.electrodragon.com/w/ESP8266_AT-Command_firmware
[official document]: https://www.espressif.com/en/support/download/documents