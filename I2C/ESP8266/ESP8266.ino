/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
    
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connect
 ion is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#define I2CAddressESPWifi 8

#include <ArduinoJson.h>

// Network parameters
const char* ssid = "MinhN";
const char* password = "Minna123";


// MQTT service
const char* mqtt_server = "192.168.0.103";
//const char* mqtt_server = "10.100.30.1";

const char* mqtt_user = "minhminh";
const char* mqtt_password = "mosquitto";

// Wifi communication
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];

unsigned int pos = 0;
unsigned int dir = 0;
unsigned int s1 = 0;
unsigned int s2 = 0;


char *encode_to_json(){ 
  const size_t bufferSize = JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject & JSONencoder = jsonBuffer.createObject();
  JSONencoder["position"] = pos;
  if (dir == 0){
    JSONencoder["status"] = "stop";
  }
  else if(dir == 1){
    JSONencoder["status"] = "closing";
  }
  else if (dir == 2){
    JSONencoder["status"] = "opening";
  }

  
  JsonArray& temperature = JSONencoder.createNestedArray("pressures");
  temperature.add(s1);
  temperature.add(s2);
  
  //JSONencoder.printTo(Serial);
  char JSONmessage[100];
  JSONencoder.printTo(JSONmessage, sizeof(JSONmessage));
//  if (client.publish("planting_room/data/module001", JSONmessage) == true){
//    Serial.println(" Success sending message");
//  }
//  else {
//    Serial.println(" Error senging message");
//  }
  return JSONmessage;
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  Wire.begin(0,2);
  Wire.setClockStretchLimit(40000);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
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

void i2cConnect(){
  Wire.beginTransmission(I2CAddressESPWifi);
  Wire.write(2);
  Wire.endTransmission();
  delay(1);
  int ava = Wire.requestFrom(I2CAddressESPWifi, 7);
  Serial.print("Request Return:[");
  unsigned int i = 0;
  byte msg[7] = {0};
  if (ava == 7){
    while(Wire.available()){
      byte c = Wire.read();
      msg[i++] = c;
      Serial.print(c);
      Serial.print(" ");
    }
    
    s1 = (msg[0] << 8) | msg[1];
    Serial.println(s1);
    s2 = (msg[2] << 8) | msg[3];
    Serial.println(s2);
    
    dir = msg[4];
    Serial.println(dir); 
    pos = (msg[5] << 8) | msg[6];
    Serial.println(pos);
  }
  else {
    Serial.println("Unexpected number of bytes receiced.");
  }
  
  Serial.println("]");
  delay(500);

}

void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic, "test_connection") == 0){
    Serial.print("Message from server: ");
//    Serial.print(topic);
//    Serial.print("] ");
//    for (int i = 0; i < length; i++) {
//      Serial.print(payload[i]);
//    }
    if (payload[0] == '0'){
      //start = 0;
      Wire.beginTransmission(I2CAddressESPWifi);
      Wire.write(0);
      Wire.endTransmission();
      delay(1);//Wait for Slave to calculate response.
    }
    else if (payload[0] == '1'){
      //start = 1;
      Wire.beginTransmission(I2CAddressESPWifi);
      Wire.write(1);
      Wire.endTransmission();
      delay(1);//Wait for Slave to calculate response.
    }
  }

  // Switch on the LED if an 1 was received as first character
//  if ((char)payload[0] == '1') {
//    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
//    // but actually the LED is on; this is because
//    // it is acive low on the ESP-01)
//  } else {
//    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
//  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello");
      // ... and resubscribe
      client.subscribe("test_connection");
      client.subscribe("test/receive_data");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
}


void loop() {
  i2cConnect();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    //Serial.print("Publish message: ");
    // Get the json string
    char *mes = encode_to_json();
    if (client.publish("planting_room/data/module001", mes) == true){
        Serial.println("Success");
    }
    else {
      Serial.println("Error");
    }
    delay(500);
  }
  delay(1000);
}
