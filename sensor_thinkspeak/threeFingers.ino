#include <SoftwareSerial.h>

// Wifi communication
#define RX 10
#define TX 11
String AP = "aalto open";       
String PASS = "";
//String AP = "MinhN";
//String PASS="Minna123";
String API = "UHUDR5TTSZAOC1EL";  
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";

SoftwareSerial esp8266(RX,TX); 

// #### PINS ####
// Pins for motor control
int motorPin = 13;
int dirPin = 4;

// Pin number for the activation button
int button = 2;

// Pin numbers for the sensors and endstops
int sensor1 = A1;
int sensor2 = A2;
int sensor3 = A3;
int endstop = 8;


// #### PARAMETERS ####
// Min and max and position of motor (unused for now)
int min = -100;
int max = 100;
int pos = 0;

// Step size and delay. Increase step and/or lower delay for faster movement.
int step = 10;
int wait = 2;

// Pressure sensor threshold (in newtons) 
double pressureThreshold = 500;

bool dir = false;
bool closed = false; // for tracking if claw is now closed or open.

void setup() {
  // Init serial for debugging
  Serial.begin(9600);

  //Wifi begin
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");

  // Set pinmodes for led, button and sensors
  pinMode(button,INPUT_PULLUP);
  pinMode(dirPin, OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(sensor1,INPUT);
  pinMode(sensor2,INPUT);
  pinMode(sensor3,INPUT);
  pinMode(endstop, INPUT_PULLUP);

}

// Function for reading the pressure sensors' values.
// Return true if all readings below threshold, false if over
bool readPressureSensors(){
  double value = 0;
  for(int i = 1; i <= 3; i++){
    switch(i){
      case 1:
        value = analogRead(sensor1);
 //       Serial.print("Sensor value: "+ String(value)+" Pos: "+String(pos)+"\n");
        break;
      case 2:
        value = analogRead(sensor2);
        break;
      case 3:
        value = analogRead(sensor3);
        break;
    }
    // TODO: Do the required math/magic to change read value to something sensible
//    Serial.print("Sensor "+String(i)+" value: "+String(value)+"\n");

    int value1 = analogRead(sensor1);
    int value2 = analogRead(sensor2);
    String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(value)+"&"+"field2="+String(value2);
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);
    sendCommand("AT+CIPCLOSE=0",5,"OK");

        
    if(value > pressureThreshold){
      Serial.print("Feedback over threshold! Sensor: "+String(i)+", Value: "+String(value)+"\n");
      return false;
    }
  }
  return true;
}


bool readEndStops(){
  if(!digitalRead(endstop) && !dir){
    return false;
  }
  return true;
}

// Implement feedback here, return false if fingers should stop
bool feedback(){
  if(!readPressureSensors() || !readEndStops()){   // TODO: add calls to other reading functions to statement as more sensors are added
    return false;
  }
  else{
    return true;
  }
}

// set all servos to same position (global variable pos)
void move(){
  if(dir){
    pos += step;
    digitalWrite(dirPin, HIGH);
  }
  else{
    pos -= step;
    digitalWrite(dirPin, LOW);
  }
  for(int i = 0; i <= step ; i++){
    digitalWrite(motorPin, HIGH);
    delay(wait);
    digitalWrite(motorPin, LOW);
  }
}

// Control loop
void loop() {
  
  // Button pressed, try to grab or open
  if(digitalRead(button) == LOW){
    if(!dir){
      dir = 1;
      // Close until at maximum position or until feedback says to stop
      while(feedback()){
        move();
      }
//      closed = true;
    }
    else{
      dir = 0;
      while(feedback()){
        move();
      }
//      closed = false;
    }
  } 
}

