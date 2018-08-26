/*
### ESP8266 MQTT

 It connects to an MQTT server then:
  
  - subscribes to the topic "control", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
    
  - If the first character of the topic "control" is an which is sent to control Arduino

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

### Serial data from ESP can be observed from another Arduino with bypassing the bootloader of ATM328
    ESP   |   Uno
    Rx    -   Rx
    Tx    -   Tx
              Reset  - GND
More information can be found at https://tttapa.github.io/Pages/Arduino/ESP8266/Flashing/Flashing-With-an-Arduino.html
    

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#define I2CAddressESPWifi 8

#include <ArduinoJson.h>

// Network parameters
const char* ssid = "<your_network_name>";
const char* password = "<your_network_password>";

// MQTT service
const char* mqtt_server = "10.100.31.1";

const char* mqtt_user = "<mqtt_server>";
const char* mqtt_password = "<mqtt_password>";

// Wifi communication
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

unsigned int pos = 0;   // position of the hand
unsigned int dir = 0;   // moving direction - closing or opening
unsigned int s1 = 0;    // value from sensor 1
unsigned int s2 = 0;    // value from sensor 2
unsigned int object = 0;// presence of the object

void encodeToJSON(){ 
  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(4);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject & JSONencoder = jsonBuffer.createObject();
  JSONencoder["position"] = pos;        // position of the piston
  if (dir == 0){                        // the status of the hand - stop, closing or opening
    JSONencoder["status"] = "stop";     
  }
  else if(dir == 1){
    JSONencoder["status"] = "closing";
  }
  else if (dir == 2){
    JSONencoder["status"] = "opening";
  }

  JsonArray& temperature = JSONencoder.createNestedArray("pressures");  // pressures from the sensors
  temperature.add(s1);
  temperature.add(s2);

  if (object == 0){
    JSONencoder["object"] = "absent";
  }
  else if (object == 1){
    JSONencoder["object"] = "present";
  }

  // Econding data to Json message
  char data[200];
  JSONencoder.printTo(data, sizeof(data)); // encode to Json
  if (client.publish("data", data) == true){  // publish the message containing data to MQTT sever
        Serial.println("Success");
  }
  else {
      Serial.println("Error");
  }
}

void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);       // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);                 // Initialize serial data for monitoring
  Wire.begin(0,2);                      // Initialize i2c connection Wire.begin(int sda, int scl), ESP8266 only functions as master
  Wire.setClockStretchLimit(40000);     // Increase the waiting time to solve problem of 
                                        // slow Arduino 
                                        // https://forum.arduino.cc/index.php?topic=336543.0
                                        // http://www.digole.com/forum.php?topicID=777
  setupWifi();
  client.setServer(mqtt_server, 1883);  // Initialize mqtt service
  client.setCallback(callback);
}

void setupWifi() {

  delay(10);
  // Start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectWifi() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe("control");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void i2cRequestData(){
  Wire.beginTransmission(I2CAddressESPWifi);
  Wire.write(5);
  Wire.endTransmission();
  delay(1);   // Wait for Arduino slave to process data
  int dataAvailable = Wire.requestFrom(I2CAddressESPWifi, 8);
  
  unsigned int i = 0;
  byte msg[8] = {0};
  if (dataAvailable == 8){
//    Serial.print("Request Return:[");
    while(Wire.available()){
      byte c = Wire.read();
      msg[i++] = c;
//      Serial.print(c);
//      Serial.print(" ");
    }
    
    s1 = (msg[0] << 8) | msg[1];
//    Serial.println(s1);
    s2 = (msg[2] << 8) | msg[3];
//    Serial.println(s2);
    
    dir = msg[4];
//    Serial.println(dir); 
    pos = (msg[5] << 8) | msg[6];
//    Serial.println(pos);
//    Serial.println("]");
    object = msg[7];
  }
  else {
    Serial.println("Unexpected number of bytes receiced.");
  } 
  delay(500);
}

void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic, "control") == 0){
//    Serial.print("Message from server: ");
//    Serial.print(topic);
//    Serial.print("] ");
//    for (int i = 0; i < length; i++) {
//      Serial.print(payload[i]);
//    }
    if (payload[0] ){
      //start = 0;
      Wire.beginTransmission(I2CAddressESPWifi);
      Wire.write(payload[0] - '0');
      Wire.endTransmission();
      delay(1);//Wait for Slave to calculate response.
    }
  }
}

void loop(){
  // Requesting data from slave
  i2cRequestData();

  if (!client.connected()) {
    reconnectWifi();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {

    encodeToJSON();

    delay(500);
  }
  delay(100);
}
