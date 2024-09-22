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
    =SER -> D5
    =OE -> D6
    =RCLK -> D7
    =SRCLK -> D8
    =SRCLR -> -[|10 Om|]- -> Vcc
    
    =Q1...Q3 -> ActiveOUTs
*/


class SN74HC595N{
  public:
    byte SER  = 5;
    byte OE   = 6;
    byte RCLK = 7;
    byte SRCLK= 8;
    void init(byte SER, byte OE, byte RCLK, byte SRCLK){
      pinMode(SER, OUTPUT);
      pinMode(OE, OUTPUT);
      pinMode(RCLK, OUTPUT);
      pinMode(SRCLK, OUTPUT);
      set_all(LOW);
    }
  
    void shift(bool zero_state = LOW, bool only = true){
      digitalWrite(OE, HIGH);
      digitalWrite(SER, zero_state);
      digitalWrite(SRCLK, HIGH);
      digitalWrite(SRCLK, LOW);
      digitalWrite(SER, LOW);
      digitalWrite(RCLK, HIGH);
      digitalWrite(OE, only);
  }
/*
    void feedStates(){

    }
*/
    void set_all(bool state = LOW){
    for (byte i=0; i<8; i++) {
      shift(state, false);
    }
  }

};