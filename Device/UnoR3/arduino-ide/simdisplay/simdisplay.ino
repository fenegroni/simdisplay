/*
   simdisplay -

   The LiquidCrystal library works with all LCD displays that are compatible with the
   Hitachi HD44780 driver. There are many of them out there, and you
   can usually tell them by the 16-pin interface.

   The circuit:
   LCD RS pin to digital pin 7
   LCD Enable pin to digital pin 8
   LCD D4 pin to digital pin 9
   LCD D5 pin to digital pin 10
   LCD D6 pin to digital pin 11
   LCD D7 pin to digital pin 12
   LCD R/W pin to ground
   LCD VSS pin to ground
   LCD VCC pin to 5V
   10K resistor:
   ends to +5V and ground
   wiper to LCD VO pin (pin 3)
*/

#include <inttypes.h>

#include <LiquidCrystal.h>

// We use the display's 4 bit interface during development.
// If we have more pins available we can make it faster
// by switching to the 8 bit interface.
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // TODO: make it clearer how we are initialising the library.


struct SimDisplayPacket {
  byte status, gear, tc, tcc, abs, bb, map, remlaps, airt, roadt;
  int16_t rpms;
} __attribute__((packed));

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop()
{
  char str[30];
  SimDisplayPacket packet;
  
  if (Serial.available()) {
    unsigned long time = micros();
    Serial.readBytes((byte *)&packet, sizeof(packet));
    sprintf(str, "GEAR=%d TC=%d", packet.gear, packet.tc);
    lcd.setCursor(0, 0);
    lcd.print(str);
    lcd.setCursor(0, 1);
    sprintf(str, "RPMS=%04" PRId16, packet.rpms);
    lcd.print(str);
    time = micros() - time;
    lcd.setCursor(10, 1);
    sprintf(str, "%lu", time);
    lcd.print(str);
  }
}
