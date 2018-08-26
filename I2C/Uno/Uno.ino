#include <Wire.h>
#include <PubSubClient.h>

#define I2CAddressESPWifi 8


// #### PINS ####
// Pins for motor control
int motorPin = 12;
int disable = 13;
int directionPin = 4;       

// Pin number for the activation button
int button = 2;

// Pin numbers for the sensors and endstops
int sensor1 = A1;
int sensor2 = A2;

int endstop = 8;
int IRsensor = 7;

bool grasped = false; // indicator if the hand is running

// #### PARAMETERS ####
// Min and max and position of motor (unused for now)
int min = 0;
int max = 10000;
unsigned int pos = 0;
unsigned int calibratedPos = 3000;

// Step size and delay. Increase step and/or lower delay for faster movement.
int step = 10;
int wait = 2;
bool calibrated = false;

// Pressure sensor threshold (in newtons)
double pressureThreshold = 500;
bool dir = false;
bool closed = false;      // for tracking if claw is now closed or open.
bool idleState = false;   // The piston is at an offset position from the bottom.
bool running = false;

byte data[8];

volatile byte control = 4;  // volatile type to force the program to read the memory otherwise, some delay is required


unsigned int sen1 = 0;
unsigned int sen2 = 0;

void setup() {
  // Init serial for debugging
  Serial.begin(115200);

  Wire.begin(I2CAddressESPWifi);
  Wire.onReceive(espWifiReceiveEvent);
  Wire.onRequest(espWifiRequestEvent);

  // Set pinmodes for led, button and sensors
  pinMode(button, INPUT_PULLUP);
  pinMode(directionPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(endstop, INPUT_PULLUP);
  pinMode(IRsensor, INPUT);
  pinMode(disable, OUTPUT);
  pinMode(motorPin, OUTPUT);
  calibrate();
  calibrated = true;
  digitalWrite(disable, true);
  //pinMode(9, OUTPUT);
  //digitalWrite(9, HIGH);
}


// Message handling from esp8266
void espWifiReceiveEvent(int count)
{
  /* Receive controlling commands
   *  0 - stop the hand
   *  1 - close the hand
   *  2 - open the hand
   *  3 - crush the object
   *  4 - automatic control
  */
  Serial.print("Control [");
  byte c;
  while (Wire.available())
  {
    c = Wire.read();
    //Serial.print(c);
  }
  
  if (c == 5){    //
    serializeData();
  }
  else {
    control = c;
    Serial.print(control);
  }
  Serial.println("]");
}

void serializeData(){
  
  int sen1 = analogRead(sensor1);
  int sen2 = analogRead(sensor2);
  data[0] = (sen1 >> 8) & 0xFF;
  data[1] = sen1 & 0xFF;
  data[2] = (sen2 >> 8) & 0xFF;
  data[3] = sen2 & 0xFF;
  if (running && dir){
    data[4] = 1 & 0xFF;
  }
  else if (running && !dir){
    data[4] = 2 & 0xFF;
  }
  else if(!running){
    data[4] = 0 & 0xFF;
  }
  
  data[5] = (pos >> 8) & 0xFF;
  data[6] = pos & 0xFF;
  data[7] = readIRSensor() & 0xFF;
//  Serial.print(sen1);
//  Serial.print(" ");
//  Serial.print((data[0] << 8) | data[1]);
//  Serial.print("  ");
//  Serial.print(sen2);
//  Serial.print(" ");
//  Serial.print((data[2] << 8) | data[3]);
//  Serial.print("  ");
//  Serial.print((data[4]));
//  Serial.print("  ");
//  Serial.print(pos);
//  Serial.print(" ");
//  Serial.println((data[5] << 8) | data[6]);
}

void espWifiRequestEvent()
{
  // Send information from hand controller to ESP
  Wire.write(data, 8);
}

// Function for reading the pressure sensors' values.
// Return true if all readings below threshold, false if over
bool readPressureSensors() {
  double value = 0;
  for (int i = 1; i <= 2; i++) {
    switch (i) {
      case 1:
        value = analogRead(sensor1);
        //Serial.print("Sensor value: "+ String(value)+" Pos: "+String(pos)+"\n");
        break;
      case 2:
        value = analogRead(sensor2);
        break;
    }   
    //    Serial.print("Sensor "+String(i)+" value: "+String(value)+"\n");
    if (value > pressureThreshold) {
      Serial.print("Feedback over threshold! Sensor: " + String(i) + ", Value: " + String(value) + "\n");
      return false;
    }
  }
  return true;
}

// Read the endstop status. Return false if piston should stop
bool readEndStops() {
  if (!digitalRead(endstop) && !dir) {
    return false;
  }
  return true;
}

// Start grasping if an object is detected by the IR sensor. Return false if the piston should stop
bool readIRSensor() {
  if (!digitalRead(IRsensor)) {
    return true;
  }
  return false;
}

// Implement feedback here, return false if fingers should stop
bool feedback() {
  if (!readPressureSensors()) {
    grasped = 1;
    return false;
  }
  else if (!readEndStops()) {
    return false;
  }
  return true;
}

// set stepper to (global variable pos)
void move() {
  digitalWrite(disable,false);
  running = true;
  if (dir && calibrated && pos < calibratedPos) {
    // The piston run to offset position
    idleState = false;
    pos += step;
    digitalWrite(directionPin, HIGH);
  }
  else if (dir && calibrated && pos == calibratedPos) {
    // The piston stop at the offset position
    idleState = true;
    pos += step;
  }
  else if (dir && calibrated && pos > calibratedPos) {
    // The piston runs to grasp the object
    idleState = false;
    pos += step;
    digitalWrite(directionPin, HIGH);
  }
  else if (!dir && !calibrated) {
    // The piston runs calibration
    idleState = false;
    grasped = false;
    pos -= step;
    digitalWrite(directionPin, LOW);
  }
  else if (!dir && calibrated) {
    // The piston runs to the offset position
    if (pos >= 3000) {
      idleState = false;
      grasped = false;
      pos -= step;
      digitalWrite(directionPin, LOW);
    }
    else {
      digitalWrite(disable, true);
      idleState = true;
      grasped = false;
    }
  }
  
  for (int i = 0; i <= step ; i++) {
    // Run the motor
    digitalWrite(motorPin, HIGH);
    delay(wait);
    digitalWrite(motorPin, LOW);
  }
}

void calibrate() {  
  // Calibrating the system, stop when reaching the endstop
  dir = false;
  while (readEndStops()) {
    Serial.println("calibrating");
    move();
  }
  digitalWrite(disable, true);
  running = false;
  delay(1000);
  pos = 0;
}

bool checkpos(){
  // Check whether the maximum position is reached
  if(pos < max){
    return true;
  }
  //grasped = true;
  return false;
}

// Control loop
void loop() {

  if (control == 1){   // close the hand command from server
    dir = 1;
    //digitalWrite(disable, false);
    while (feedback() && checkpos() && control == 1){
      //Serial.print("");//Why does this make it work?
      move();
    }
  }
  else if(control == 2){    // open the hand command from server
    dir = 0;    
    while (!idleState && control == 2){
      //Serial.print("");//Why does this make it work?
      move();
    }
  }
  else if (control == 3){       // crush stuff
    dir = 1;
    while(checkpos() && control == 3){
      move();
    }
  }
  else if (control == 4) {  // Self control 
    // Detecting object
    if(readIRSensor()){
//      Serial.print("IR ");
//      Serial.print(readirectionPinSensor());
      
      dir = 1;
      
      // If an object presnts, the piston closes until at maximum position or until feedback says to stop
      while(feedback() && readIRSensor() && !grasped && checkpos() && control == 4){
        idleState = false;
        move();
      }

      // When the object is already grasped, press the button to release the fingers.
      if (digitalRead(button) == LOW && grasped){

        // CRUSH CODE
        for(int i = 0; !digitalRead(button) ; i++){
          delay(50);
          if(i > 20){
            Serial.print("Would you kindly release the button to...\n");
            Serial.print("CRUSH, MAIM, DESTROY!\n");
            while(!digitalRead(button)){ /* DO NOTHING */}
            //digitalWrite(disable,false);
            Serial.print("Crushing...\n");
            while(checkpos() && digitalRead(button) && control == 4){
              move();
            }
            Serial.print("Object crushed!\n");
            while(digitalRead(button)){ /* DO NOTHING */}
            break;
          }
        }
        // END OF CRUSH CODE
     
        directionPin = 0;
        while(!idleState && control == 4){
          move();
        }
      }
    }
    else {
      // Change the directionPinectionPin to move to offset position
      if (!dir){  
        dir = 1;
      }
      else if (dir){ 
        dir = 0;
      }
      
      // Move the piston to the offset position when there is no object or when the button is pressed to release the fingers
      while(!readIRSensor() && !idleState && control == 4){
        move();
      }
    }
  }
  
  digitalWrite(disable, true);
  running = false;
}
