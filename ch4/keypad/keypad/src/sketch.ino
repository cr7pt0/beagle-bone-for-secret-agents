/*
Test i2c read from RTC
Tim Steiner
 */

#include "Wire.h"
#define DS3231M 0x68 // each I2C object has a unique bus address, the DS3231M is 0x68

void setup()
{
  Wire.begin(); // wake up I2C bus
  Serial.begin(9600);
}

void gettime(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) // request the current time stored in DS3231M
{
 Wire.beginTransmission(0x68); // "Hey, RTC @ 0x68! Message for you"
 Wire.write(0); // "move your register pointer back to 00h"
 Wire.endTransmission(); // "Thanks, goodbye..."
  // now get the data from the RTC
 Wire.requestFrom(DS3231M, 7); // "Hey, RTC @ 0x68 - please send me the contents of your first seven registers"
 *second = bcdToDec(Wire.read(); // first received byte stored here
 *minute = bcdToDec(Wire.read(); // second received byte stored here
 *hour = bcdToDec(Wire.read(); // third received byte stored here
 *dayOfWeek = bcdToDec(Wire.read(); // forth received byte stored here
 *dayOfMonth = bcdToDec(Wire.read(); // fifth received byte stored here
 *month = bcdToDec(Wire.read(); // sixth received byte stored here
 *year = bcdToDec(Wire.read(); // seventh received byte stored here
 Serial.print("Time = ");
 Serial.print(month,1);
 Serial.print(dayOfMonth,1);
 Serial.print(year,1);
 Serial.print(hour,1);
 Serial.print(minute,1);
 Serial.print(second,1);
}

void loop()
{
 gettime();
 delay(5000);
}
