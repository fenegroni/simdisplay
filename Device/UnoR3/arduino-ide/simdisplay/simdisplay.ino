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

/*
 * The Uno R3 has a 64 byte buffer in the serial port.
 * TODO: make the packets a divsor of 64 to ensure no packets are ever lost
 */
struct SimDisplayPacket {
  byte status, gear, tc, tcc, abs, bb, map, remlaps, airt, roadt;
  int16_t rpms;
};

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600);

  
  lcd.setCursor(0, 0);
  lcd.print("GEAR=  TC=      ");
  lcd.setCursor(0, 1);
  lcd.print("RPMS=           ");
}

void loop()
{
  char str[30];
  SimDisplayPacket packet;

  if (Serial.available()) {
    unsigned long time = micros();
    char gear;
    Serial.readBytes((byte *)&packet, sizeof(packet));
    if (packet.gear == 0) {
      gear = 'R';
    } else if (packet.gear == 1) {
      gear = 'N';
    } else {
      gear = '0' + (packet.gear - 1);
    }
    lcd.setCursor(5, 0);
    lcd.print(gear);
    lcd.setCursor(10, 0);
    lcd.print(packet.tc);
    lcd.setCursor(5, 1);
    sprintf(str, "%04" PRId16, packet.rpms);
    lcd.print(packet.rpms);
    time = micros() - time;
    lcd.setCursor(10, 1);
    sprintf(str, "%lu", time);
    lcd.print(str);
  }
}
