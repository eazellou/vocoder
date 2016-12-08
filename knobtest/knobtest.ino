const int packetStartVal = 255;

int potPin0 = 0;    // select the input pin for the potentiometer
int potPin1 = 1;
int potPin2 = 2;
int potPin3 = 3;
int potPin4 = 4;
int potPin5 = 5;
int potPin6 = 6;
int potPin7 = 7;
int potPin8 = 8;
int val0 = 0;       // variable to store the value coming from the sensor
int val1 = 0;
int val2 = 0;
int val3 = 0;
int val4 = 0;
int val5 = 0;
int val6 = 0;
int val7 = 0;
int val8 = 0;

void setup() {
  Serial.begin(14400);
}

int clipRange(int val, int minimum, int maximum) {
  if (val < minimum) {
    return minimum;
  } else if (val > maximum) {
    return maximum;
  } else {
    return val;
  }
}

void loop() {
  val0 = analogRead(potPin0);    // read the value from the sensor
  val0 = map(val0, 0, 1023, 0, 254); //Remap value to desired range
  delay(1);
  val1 = analogRead(potPin1);    
  val1 = map(val1, 0, 1023, 0, 254);
  delay(1);
  val2 = analogRead(potPin2);    
  val2 = map(val2, 0, 1023, 0, 254);
  delay(1);
  val3 = analogRead(potPin3);    
  val3 = map(val3, 0, 1023, 0, 254);
  delay(1);
  val4 = analogRead(potPin4);    
  val4 = map(val4, 0, 1023, 0, 254);
  delay(1);
  val5 = analogRead(potPin5);    
  val5 = map(val5, 0, 1023, 0, 254);
  delay(1);
  val6 = analogRead(potPin6);    
  val6 = map(val6, 0, 1023, 0, 254);
  delay(1);
  val7 = analogRead(potPin7);    
  val7 = map(val7, 0, 1023, 0, 254);
  delay(1);
  // 8 doesn't work so actually is 10
  val8 = analogRead(potPin8);    
  val8 = map(val8, 0, 1023, 0, 254);
  delay(1);

  write_packet();
  //delay(10);
}

void write_packet(){
   Serial.write(0xFF);
   Serial.write(val0);
   Serial.write(val1);
   Serial.write(val2);
   Serial.write(val3);
   Serial.write(val4);
   Serial.write(val5);
   Serial.write(val6);
   Serial.write(val7);
   Serial.write(val8);
}
