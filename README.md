# PISTON HAND 
## I2C and MQTT features for wireless communication 


These features aim to provide the piston hand with more control flexibility and 
communication. They are possible to be used in IoT applications or serve as a data 
transmission tool for "digital twin". 


## I2C between Arduino Uno and ESP8266
In this project, wifi module ESP8266-01 module is used. 
I2C connection between Arduino and ESP8266 can be established 
using the following [connections].

 Uno  |  ESP  
:---: | :---: 
  SDA |  GPIO0
  SCL |  GPIO2
  GND |  GND
For observing the ESP8266, serial of another Arduino board can be used and connected as 

  Uno  |  ESP  
:---:  | :---: 
  Tx   |  Tx
  Rx   |  Rx
  Reset - GND |
 The reset pin of the second Arduino is connected to ground to bypass the bootloader of 
 the ATM328P and using USB port of the ATM16U2 on the board. 
 More information can be found at <https://tttapa.github.io/Pages/Arduino/ESP8266/Flashing/Flashing-With-an-Arduino.html>.
 ![alt text](https://tttapa.github.io/Pages/Arduino/ESP8266/Flashing/images/ESP8266-UNO-flash.svg)

## MQTT between ESL8266 and a server



## Problems and Troubleshooting

```
Wire.begin(0,2);                      // Initialize i2c connection Wite.begin(int sda, int sck), ESP8266 only functions as master
```
```
 Wire.setClockStretchLimit(40000);     // Increase the waiting time to solve problem of
```
https://forum.arduino.cc/index.php?topic=336543.0
http://www.digole.com/forum.php?topicID=777



[connections]: https://arduino-esp8266.readthedocs.io/en/latest/libraries.html