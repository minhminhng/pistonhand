// #### PINS ####
// Pins for motor control
int motorPin = 12;
int disable = 13;     // activate so that the motor does not run when it is at idle state
int dirPin = 4;       // dir = 0 for openning the hand, dir = 1 for closing the hand

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
int max = 100;
int pos = 0;

// Step size and delay. Increase step and/or lower delay for faster movement.
int step = 10;
int wait = 2;
bool calibrated = false;

// Pressure sensor threshold (in newtons) 
double pressureThreshold = 750;

bool dir = false;
bool closed = false;      // for tracking if claw is now closed or open.
bool idleState = false;   // The piston is at an offset position from the bottom.

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

// Start grasping if an object is detected by the IR sensor. Return false if the piston should stop
bool readIRSensor(){
  if (!digitalRead(IRsensor)){
    return true;
  }
  return false;
}

// Implement feedback here, return false if fingers should stop
bool feedback(){
  if(!readPressureSensors() || !readEndStops()){   // TODO: add calls to other reading functions to statement as more sensors are added
    grasped = 1;
    return false;
  }
  return true;
}

// set stepper to (global variable pos)
void move(){
  if(dir && calibrated && pos < 3000){
    // The piston runs to grasp the object
    idleState = false;
    pos += step;
    digitalWrite(dirPin, HIGH);
  }
  else if(dir && calibrated && pos >= 3000 && pos < 3005){
    idleState = true;
    pos += step;
  }
  else if(dir && calibrated && pos > 3005){
    idleState = false;
    pos += step;
    digitalWrite(dirPin, HIGH);
  }
  else if (!dir && !calibrated){
    // The piston runs calibration
    idleState = false;
    grasped = false;
    pos -= step;
    digitalWrite(dirPin, LOW);
  }
  else if(!dir && calibrated){
    // The piston runs to the offset position
    if (pos >=3000){
      idleState = false;
      grasped = false;
      pos -= step;
      digitalWrite(dirPin, LOW);
    }
    else{
      digitalWrite(disable,true);
      idleState = true;
      grasped = false;
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

bool buttonPressed(){
  if (digitalRead(button) == LOW){
    if (dir == 1){
      dir = 0;
    }
    else {
      dir = 1;
    }
  }
}

// Control loop
void loop() {
 
    // Detecting object
    if(readIRSensor()){
      Serial.print("IR ");
      Serial.print(readIRSensor());
      
      dir = 1;
      
      // Close until at maximum position or until feedback says to stop
      while(feedback() && readIRSensor() && !grasped){
        idleState = false;
        digitalWrite(disable,false);
        move();
      }

      if (digitalRead(button) == LOW && grasped){
        digitalWrite(disable,false);
        dir = 0;
        while(!idleState){
          move();
        }
      }
      digitalWrite(disable,true);
    }
    else {
      if (!dir){  
        dir = 1;
      }
      else if (dir){ 
        dir = 0;
      }
      digitalWrite(disable,false);
      //Serial.println(dir);
      Serial.println(pos);
      while(!readIRSensor() && !idleState){
        move();
      }
      digitalWrite(disable,true);
    }
}
