// #### PINS ####
// Pins for motor control
int motorPin = 12;
int disable = 13;     // activate so that the motor does not run when it is at idle state
int dirPin = 4;       // dir = 0 for openning the hand, dir = 1 for closing the hand

// Pin number for the activation button
int button = 2;

// Pin numbers for the sensors and endstops
int sensor1 = A1;     // pressure sensor
int sensor2 = A2;     //pressure sensor
  
int endstop = 8;      
int IRsensor = 7;


bool grasped = false; // indicator if the hand is running 

// #### PARAMETERS ####
// Min and max and position of motor (unused for now)
int min = 0;
int max = 9500;
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
  if(!readPressureSensors()){
    grasped = 1;
    return false;
  }
  else if(!readEndStops()){   // TODO: add calls to other reading functions to statement as more sensors are added
    return false;
  }
  return true;
}

// set stepper to (global variable pos)
void move(){
  
  if(dir && calibrated && pos < 3000){
    // The piston run to offset position
    idleState = false;
    pos += step;
    digitalWrite(dirPin, HIGH);
  }
  else if(dir && calibrated && pos >= 3000 && pos < 3005){
    // The piston stop at the offset position
    idleState = true;
    pos += step;
  }
  else if(dir && calibrated && pos > 3005){
    // The piston runs to grasp the object
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
//  Serial.print("Pos: "+String(pos)+"\n");
}

void calibrate(){
  dir = false;
  while(readEndStops()){
 //   Serial.print("read end stop");
 //   Serial.println(readEndStops());
        move();
  }
  delay(1000);
  pos = 0;
}

bool checkpos(){
  if(pos < max){
    return true;
  }
  grasped = true;
  return false;
}

// Control loop
void loop() {
    
    
    // Detecting object
    if(readIRSensor()){
//      Serial.print("IR ");
//      Serial.print(readIRSensor());
      
      dir = 1;
      
      // If an object presnts, the piston closes until at maximum position or until feedback says to stop
      while(feedback() && readIRSensor() && !grasped && checkpos()){
        idleState = false;
        digitalWrite(disable,false);
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
            digitalWrite(disable,false);
            Serial.print("Crushing...\n");
            while(checkpos() && digitalRead(button)){
              move();
            }
            Serial.print("Object crushed!\n");
            while(digitalRead(button)){ /* DO NOTHING */}
            break;
          }
        }
        // END OF CRUSH CODE
        
        digitalWrite(disable,false);
        dir = 0;
        while(!idleState){
          move();
        }
      }
      digitalWrite(disable,true);
    }
    else {
      // Change the direction to move to offset position
      if (!dir){  
        dir = 1;
      }
      else if (dir){ 
        dir = 0;
      }
      digitalWrite(disable,false);
      
      // Move the piston to the offset position when there is no object or when the button is pressed to release the fingers
      while(!readIRSensor() && !idleState){
        move();
      }
      digitalWrite(disable,true);
    }
}
