#include <Wire.h>
#include <I2C_RTC.h>

static DS3231 RTC;

void setup()
{
  Serial.begin(115200);
  RTC.begin();

  RTC.setHourMode(CLOCK_H24);

  RTC.setDate(10,10,2029);
  RTC.setTime(12,9,10);


}

void loop()
{

  Serial.print(" ");
  Serial.print(RTC.getDay());
  Serial.print("-");
  Serial.print(RTC.getMonth());
  Serial.print("-");
  Serial.print(RTC.getYear());

  Serial.print(" ");

  Serial.print(RTC.getHours());
  Serial.print(":");
  Serial.print(RTC.getMinutes());
  Serial.print(":");
  Serial.print(RTC.getSeconds());
  Serial.println(" ");

  delay(1000);
}
