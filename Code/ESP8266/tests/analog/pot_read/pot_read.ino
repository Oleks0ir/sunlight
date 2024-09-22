#define pot_pin A0

String pot_value = "Value";

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(pot_pin, INPUT);

}

String readpot(){
    pot_value = String(analogRead(pot_pin));
    Serial.println(pot_value);
    return pot_value;
}


void loop() {
  // put your main code here, to run repeatedly:
  readpot();
}
