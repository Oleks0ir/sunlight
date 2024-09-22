/*
A library used t communicate with the shift register SN74HC595N
        Pinout:
          ________
 Q1   1 -|        |- 16 Vcc
 Q2   2 -|        |- 15 Q0
 Q3   3 -|        |- 14 SER (Serial Data Input)
 Q4   4 -|        |- 13 OE (Output Enable, active low)
 Q5   5 -|        |- 12 RCLK (Storage Register Clock, Latch)
 Q6   6 -|        |- 11 SRCLK (Shift Register Clock)
 Q7   7 -|        |- 10 SRCLR (Shift Register Clear, active low)
GND   8 -|________|-  9 Q7' (Serial Out)

The standart Pinout is next:
    =GND -> GND
    =GND -> -||- (1 nF) -> Vcc    
    =Vcc -> 3,3V   
    =SER -> D5  ->14
    =OE -> D6 ->12
    =RCLK -> D7 ->13
    =SRCLK -> D8 ->15
    =SRCLR -> -[|10 Om|]- -> Vcc
    
    =Q1...Q3 -> ActiveOUTs
*/
// Pin definitions for ESP8266
#define dataPin  14   // SER connected to D5
#define latchPin  13  // RCLK connected to D7
#define clockPin  15  // SRCLK connected to D8
#define outputEnablePin  12 // OE connected to D6
#define Vin A0

void setup() {
  Serial.begin(115200);
  // Set pin modes
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(outputEnablePin, OUTPUT);
  pinMode(Vin, INPUT);

  // Set initial states
  digitalWrite(outputEnablePin, LOW); // Enable output
  digitalWrite(latchPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(dataPin, LOW);

  Serial.println("Started");
}

void loop() {
  // Example signal list: Change this list as needed
  int signals[] = {0, 0, 0, 0, 0, 1};

  // Update the shift register with the signal list
  updateShiftRegister(signals, sizeof(signals) / sizeof(signals[0]));

  delay(3000); // Wait for a second before updating again
  Serial.println("-----------------------------------------------");
}

void updateShiftRegister(int* signals, int length) {
  // Shift out each bit of the signals list
  
  for (int i = length - 1; i >= 0; i--) {
    digitalWrite(latchPin, LOW);
    
    digitalWrite(clockPin, LOW);
    digitalWrite(dataPin, signals[i] ? HIGH : LOW);
    digitalWrite(clockPin, HIGH);
    
    digitalWrite(latchPin, HIGH);
      if (i< length-1 && i>0){
        Serial.println(analogRead(Vin));
      delay(1000);
      }
  }

  // Set latchPin to HIGH to update the output of the shift register
}
