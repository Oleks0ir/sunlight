#define M0 13
#define M1 12
#define M2 14
int i = 0;

void setup() {
  // put your setup code here, to run once:
  declarePinModes();
  Serial.begin(115200);
  eraseMult();
  digitalWrite(M0, HIGH);
}

void loop() {
/*
  // put your main code here, to run repeatedly:
    switchMult_on(i%8);
    delay(1000);
    Serial.print("\n");Serial.print(i);Serial.print(" | ");Serial.print(i%7);
    i++;
    */
}

void declarePinModes(){
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
}

void switchMult_on(int channel){
  eraseMult();

  int M0_state = LOW;
  int M1_state = LOW;
  int M2_state = LOW;

  if(channel%2 == 1){
    M0_state = HIGH;
    Serial.print(" | M0: ");Serial.print(i%8);
  }
  
  if(channel%8 >8){
    M2_state = HIGH;
    Serial.print(" | M2: ");Serial.print(i%8);
    channel-=1;
  }

  if(channel%4 >2){
    Serial.print(" | M1: ");Serial.print(i%8);
    M1_state = HIGH;
  }



  digitalWrite(M0, M0_state);
  digitalWrite(M1, M1_state);
  digitalWrite(M2, M2_state);
}

void eraseMult(){
  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
}