void setup() {
Serial.begin(115200);
pinMode(8, INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(digitalRead(8));
}
