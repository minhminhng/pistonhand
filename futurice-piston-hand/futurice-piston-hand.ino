// #### PINS ####
// Pins for motor control
int motorPin = 12;
int disable = 13;
int dirPin = 4;

// Pin number for the activation button
int button = 2;

// Pin numbers for the sensors and endstops
int sensor1 = A1;
int sensor2 = A2;
int sensor3 = A3;
int endstop = 8;
int IRsensor = 7;

int running = 0;
bool grasp = true; // indicator if the hand is ready to grasp

// #### PARAMETERS ####
// Min and max and position of motor (unused for now)
int min = 0;
int max = 100;
int pos = 0;

// Step size and delay. Increase step and/or lower delay for faster movement.
int step = 10;
int wait = 2;
bool calibrated = false;

// Pressure sensor threshold (in newtons) 
double pressureThreshold = 750;

bool dir = false;
bool closed = false; // for tracking if claw is now closed or open.

void setup() {
  // Init serial for debugging
  Serial.begin(9600);

  // Set pinmodes for led, button and sensors
  pinMode(button,INPUT_PULLUP);
  pinMode(dirPin, OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(sensor1,INPUT);
  pinMode(sensor2,INPUT);
  pinMode(endstop, INPUT_PULLUP);
  pinMode(IRsensor, INPUT);
  pinMode(disable, OUTPUT);
  calibrate();
  calibrated = true;
  digitalWrite(disable, true);
}

// Function for reading the pressure sensors' values.
// Return true if all readings below threshold, false if over
bool readPressureSensors(){
  double value = 0;
  for(int i = 1; i <= 2; i++){
    switch(i){
      case 1:
        value = analogRead(sensor1);
 //       Serial.print("Sensor value: "+ String(value)+" Pos: "+String(pos)+"\n");
        break;
      case 2:
        value = analogRead(sensor2);
        break;
    }
    // TODO: Do the required math/magic to change read value to something sensible
//    Serial.print("Sensor "+String(i)+" value: "+String(value)+"\n");
    if(value > pressureThreshold){
      Serial.print("Feedback over threshold! Sensor: "+String(i)+", Value: "+String(value)+"\n");
      return false;
    }
  }
  return true;
}



// Read the endstop status. Return false if piston should stop
bool readEndStops(){
  if(!digitalRead(endstop) && !dir){
    return false;
  }
  return true;
}

// Start grasping if an object is detected by the IR sensor
bool readIRSensor(){
  if (!digitalRead(IRsensor)){
    return true;
  }
  return false;
}

// Implement feedback here, return false if fingers should stop
bool feedback(){
  if(!readPressureSensors() || !readEndStops()){   // TODO: add calls to other reading functions to statement as more sensors are added
    return false;
  }
  return true;
}

// set stepper to (global variable pos)
void move(){
  if(dir){
    pos += step;
    digitalWrite(dirPin, HIGH);
  }
  else if (!dir && !calibrated){
    pos -= step;
     digitalWrite(dirPin, LOW);
  }
  else if(!dir && calibrated){
    if (pos >=3000){
      pos -= step;
      digitalWrite(dirPin, LOW);
    }
    else{
      digitalWrite(disable,true);
      running = 0;
    } 
  }
  
  for(int i = 0; i <= step ; i++){
    digitalWrite(motorPin, HIGH);
    delay(wait);
    digitalWrite(motorPin, LOW);
  }
}

void calibrate(){
  dir = false;
  while(readEndStops()){
        move();
  }
  pos = 0;
}

// Control loop
void loop() {
  // Button pressed, try to grab or open
    boolean IRs = readIRSensor();
    if(digitalRead(button) == LOW){
      if (dir == 1){
        dir = 0;
        move();
        else {
          dir = 1;
        }
      if(IRs){
      Serial.print("IR ");
      Serial.print(IRs);
      digitalWrite(disable,false);
  
  
        // Close until at maximum position or until feedback says to stop
        while(feedback() && readIRSensor()){
          move();
        }
      }
//      else{
//        dir = 0;
//        while(feedback() ){
//          move();
//        }
//      }
      digitalWrite(disable,true);
      running = 0;
    }
    else {
      dir = 0;
      digitalWrite(disable,false);
      running = 1;
      while(!readIRSensor() && readEndStops()){
        move();
      }
      digitalWrite(disable,true);
    }
}
