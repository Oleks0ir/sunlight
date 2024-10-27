#define Switch 14 //D5          


void setup() {
  // put your setup code here, to run once:
  pinMode(Switch, OUTPUT);
  Serial.begin(115200);
  Serial.println(readVoc());
  Serial.println(readV_lowRes());
  

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(readVoc());
  delay(2000);
  Serial.println(readV_lowRes());
  delay(2000);
  
}


String readVoc(){
  unsigned int value;
  digitalWrite(Switch, LOW);
  delay(5);
  value = analogRead(A0);
  Serial.print("Voltage OC  = " + String(value, HEX) + " | " + String(value*3.22265/1000) + "V");
  //Serial.println((int)(value*100));
  return String((int)(value), HEX);
}

String readV_lowRes(){
  unsigned int value;
  digitalWrite(Switch, HIGH);
  delay(5);
  value = analogRead(A0);
  Serial.print("Voltage LRES = " + String(value, HEX) + " | " + String(value*3.22265) + " mV");
  //Serial.println((int)(value*100));
  return String((int)(value), HEX);
}
