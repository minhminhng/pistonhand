# PISTON HAND 
## I2C and MQTT features for wireless communication 


These features aim to provide the piston hand with more control flexibility and 
communication. They are possible to be used in IoT applications or serve as a data 
transmission tool for "digital twin". 


## I2C between Arduino Uno and ESP8266
In this project, wifi module ESP8266-01 module is used. 
I2C connection between Arduino and ESP8266 can be established 
using the following [connections].

 Uno  |  ESP8266-01  
:---: | :---: 
  SDA |  GPIO0
  SCL |  GPIO2
  GND |  GND
  
For observing the ESP8266, serial of another Arduino board can be used and connected as 

  Uno  |  ESP8266-01 
:---:  | :---: 
  Tx   |  Tx
  Rx   |  Rx
  Reset - GND |
  
 The reset pin of the second Arduino is connected to ground to bypass the bootloader of 
 the ATM328P and using USB port of the ATM16U2 on the board. This connection can also be 
 used for uploading sketch to ESP8266 module via Arduino Uno.
 More information can be found at <https://tttapa.github.io/Pages/Arduino/ESP8266/Flashing/Flashing-With-an-Arduino.html>.
 ![alt text](https://tttapa.github.io/Pages/Arduino/ESP8266/Flashing/images/ESP8266-UNO-flash.svg)

Then, in ESP8266 code, the following code is used for initilize the I2C communication:

```
#define I2CAddressESPWifi 8
...
void setup(){
    Wire.begin(0,2);   // Initialize i2c connection Wire.begin(int sda, int scl)
    ...
}
...
```
! _Note_: ESP8266 only functions as a master and the address should be at least 6.

In Arduino code, the following code is used for establishing the connection:


```
#define I2CAddressESPWifi 8
...
void setup(){
    Wire.begin(I2CAddressESPWifi);
    ...
}
...
```
## MQTT between ESP8266 and a server
To establish the MQTT communication, the following parameters are loaded to ESP8266:
```
// Network parameters
const char* ssid = "<your_network_name>";
const char* password = "<your_network_password>";

// MQTT service
const char* mqtt_server = "<your_server_IP>";

const char* mqtt_user = "<mqtt_server>";
const char* mqtt_password = "<mqtt_password>";
```

## Problems and Troubleshooting

A common problem of I2C communication is Arduino is slower than ESP8266. 
When data is send as a single byte, the problem is almost unnoticeable. 
However, when a large number of bytes are sent, they can be come junk. 
The following steps can be used to solve the problem. 
Firstly, increase the waiting time of ESP8266
```
 Wire.setClockStretchLimit(40000);     // Increase the waiting time to solve problem of
```
Secondly, send a command so that Arduino can prepare the data:
* ESP code:
```
void i2cRequestData(){
  Wire.beginTransmission(I2CAddressESPWifi);
  Wire.write(5);
  Wire.endTransmission();
  delay(1);   // Wait for Arduino slave to process data
  ...
}
```
* Arduino code:

```
byte data[number_of_bytes] = {0};
void setup(){
    ...
    Wire.onRequest(espWifiRequestEvent);
    ...
}
void serializeData(){
    // Processing your data here
    data[0] = ...;
    data[1] = ...;
    ...
}
void espWifiRequestEvent()
{
  // Send information from hand controller to ESP
  Wire.write(data, number_of_bytes);
}
```
More discussion can be found at <https://forum.arduino.cc/index.php?topic=336543.0> and 
<http://www.digole.com/forum.php?topicID=777>.

Another problem in Arduino code is the change of the variable sent from ESP8266 sometimes 
can not be detected although it is a global variable. In this case, the variable _control_
was the problem. This can be solved by declaring it as a _**[volatile]**_ variable:
```
volatile byte control = 0;
```

[connections]: https://arduino-esp8266.readthedocs.io/en/latest/libraries.html
[volatile]: http://gammon.com.au/interrupts