/*
  Use for testing proper function of motor driver
*/

/* Without PWM */
//const int dirPin = 4;
//const int driverPin = 12;
//void setup() {
//  // initialize digital pin LED_BUILTIN as an output.
//  pinMode(dirPin, OUTPUT);
//  pinMode(driverPin, OUTPUT);
//}
//
//// the loop function runs over and over again forever
//void loop() {
//  // manualy drive the motor
//  digitalWrite(dirPin, HIGH);     
//  //digitalWrite(dirPin, LOW;
//  digitalWrite(driverPin, HIGH);
//  delay(1);
//  digitalWrite(driverPin, LOW);
//  delay(1);
//}
/* With PWM */
 const int dirPin = 4;
 const int driverPin = 5;
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(dirPin, OUTPUT);
  pinMode(driverPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  // manualy drive the motor
  digitalWrite(dirPin, HIGH);     
  //digitalWrite(dirPin, LOW);
  analogWrite(driverPin , 254);
}
