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

#include "SimDisplayProtocol.h"

#include <LiquidCrystal.h>

// We use the display's 4 bit interface during development.
// If we have more pins available we can make it faster
// by switching to the 8 bit interface.
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // TODO: make it clearer how we are initialising the library.

/*
 * The Uno R3 has a 64 byte buffer in the serial port.
 * TODO: make the packets a divisor of 64 to ensure no packets are ever lost if that can help
 */

struct SimDisplayPacket packet[2];
struct SimDisplayPacket *newPacket, *oldPacket;
int isOldPacketUsed;

char strbuffer[17];

void resetDisplayMask()
{
  lcd.setCursor(0, 0);
  //         0123456789012345
  lcd.print("-    R--    -/--");
  lcd.setCursor(0, 1);
  //         0123456789012345
  lcd.print("--%   M-  --/--C");
}

void setup()
{
  newPacket = &packet[0];
  oldPacket = &packet[1];
  
  lcd.begin(16, 2);
  resetDisplayMask();
  Serial.begin(9600);
}

void loop()
{
  while (Serial.available() >= sizeof(struct SimDisplayPacket)) {
    unsigned long time = micros();
    if (sizeof(struct SimDisplayPacket) > Serial.readBytes((byte *)newPacket, sizeof(struct SimDisplayPacket))) {
      // something went wrong, we reset ourselves.
      Serial.end();
      delay(2000);
      setup();
      break;
    }
    if (SDP_STATUS_LIVE != newPacket->status) {
      resetDisplayMask();
      continue;
    }
    if (newPacket->abs != oldPacket->abs) {
      lcd.setCursor(0, 0);
      sprintf(strbuffer, "%" PRIu8, newPacket->abs);
      lcd.print(strbuffer);
    }
    if (newPacket->fuelEstimatedLaps != oldPacket->fuelEstimatedLaps) {
      lcd.setCursor(6, 0);
      sprintf(strbuffer, "%2" PRIu8, newPacket->fuelEstimatedLaps);
      lcd.print(strbuffer);
    }
    if (newPacket->tc != oldPacket->tc) {
      lcd.setCursor(11, 0);
      sprintf(strbuffer, "%2" PRIu8, newPacket->tc);
      lcd.print(strbuffer);
    }
    if (newPacket->tcc != oldPacket->tcc) {
      lcd.setCursor(14, 0);
      sprintf(strbuffer, "%2" PRIu8, newPacket->tcc);
      lcd.print(strbuffer);
    }
    if (newPacket->bb != oldPacket->bb) {
      lcd.setCursor(0, 1);
      sprintf(strbuffer, "%2" PRIu8, newPacket->bb);
      lcd.print(strbuffer);
    }
    if (newPacket->engineMap != oldPacket->engineMap) {
      lcd.setCursor(7, 1);
      sprintf(strbuffer, "%1" PRIu8, newPacket->engineMap);
      lcd.print(strbuffer);
    }
    if (newPacket->airTemp != oldPacket->airTemp) {
      lcd.setCursor(10, 1);
      sprintf(strbuffer, "%2" PRIu8, newPacket->airTemp);
      lcd.print(strbuffer);
    }
    if (newPacket->roadTemp != oldPacket->roadTemp) {
      lcd.setCursor(13, 1);
      sprintf(strbuffer, "%2" PRIu8, newPacket->roadTemp);
      lcd.print(strbuffer);
    }
    struct SimDisplayPacket *tmp = oldPacket;
    oldPacket = newPacket;
    newPacket = tmp;
    time = micros() - time;
  }
}
