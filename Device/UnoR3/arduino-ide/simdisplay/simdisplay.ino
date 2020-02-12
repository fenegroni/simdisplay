/*
simdisplay - A simracing dashboard created using Arduino to show shared memory
             telemetry from Assetto Corsa Competizione.

Copyright (C) 2020  Filippo Erik Negroni

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
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

#include "SimDisplayProtocol.h"

// ----------------------  0123456789012345
#define DISPLAY_MASK_ROW0 "-   R:--    -/- "
#define DISPLAY_MASK_ROW1 "--.-% M-  --/--C"
#define DISPLAY_ABS_COLROW      0, 0
#define DISPLAY_REMLAPS_COLROW  6, 0
#define DISPLAY_TC_COLROW       11, 0
#define DISPLAY_TCC_COLROW      14, 0
#define DISPLAY_BB_COLROW       0, 1
#define DISPLAY_MAP_COLROW      7, 1
#define DISPLAY_AIRT_COLROW     10, 1
#define DISPLAY_ROADT_COLROW    13, 1

// We use the display's 4 bit interface during development.
// If we have more pins available we can make it faster
// by switching to the 8 bit interface.
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // TODO: make it clearer how we are initialising the library.

static struct SimDisplayPacket packet[2];
static struct SimDisplayPacket *newPacket = &packet[0];
static struct SimDisplayPacket *oldPacket = &packet[1];

void lcdPrint(char *str, int col, int row)
{
  lcd.setCursor(col, row);
  lcd.print(str);
}

void printDisplayMask()
{
  lcdPrint(DISPLAY_MASK_ROW0, 0, 0);
  lcdPrint(DISPLAY_MASK_ROW1, 0, 1);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  lcd.begin(16, 2);
  printDisplayMask();
  Serial.begin(9600);
}

void printDisplayField(int newval, int oldval, char *zerostr, char *fmtstr, int col, int row)
{
  static char strbuffer[17];
  
  if (newval != oldval) {
    if (0 == newval) {
      strcpy(strbuffer, zerostr);
    } else {
      sprintf(strbuffer, fmtstr, newval);
    }
    lcdPrint(strbuffer, col, row);
  }
}

void printDisplayFieldBB(int newval, int oldval, int col, int row)
{
  static char strbuffer[17];
  
  if (newval != oldval) {
    if (0 == newval) {
      strcpy(strbuffer, "--.-");
    } else {
      sprintf(strbuffer, "%d.%d", newval/10, newval%10);
    }
    lcdPrint(strbuffer, col, row);
  }
}

void printDisplayFields()
{
  static char strbuffer[17];
  
  printDisplayField(newPacket->abs, oldPacket->abs, "- ", "%-2d", DISPLAY_ABS_COLROW);
  printDisplayField(newPacket->remlaps, oldPacket->remlaps, "--", "%2d", DISPLAY_REMLAPS_COLROW); 
  printDisplayField(newPacket->tc, oldPacket->tc, " -", "%2d", DISPLAY_TC_COLROW);
  printDisplayField(newPacket->tcc, oldPacket->tcc, "- ", "%-2d", DISPLAY_TCC_COLROW);
  printDisplayFieldBB(newPacket->bb, oldPacket->bb, DISPLAY_BB_COLROW);
  printDisplayField(newPacket->map, oldPacket->map, "- ", "%-2d", DISPLAY_MAP_COLROW);
  printDisplayField(newPacket->airt, oldPacket->airt, "--", "%2d", DISPLAY_AIRT_COLROW);
  printDisplayField(newPacket->roadt, oldPacket->roadt, "--", "%2d", DISPLAY_ROADT_COLROW);
}

void loop()
{
  while (Serial.available() >= sizeof(struct SimDisplayPacket)) {
    unsigned long time = micros();
    
    if (sizeof(struct SimDisplayPacket) > Serial.readBytes((byte *)newPacket, sizeof(struct SimDisplayPacket))) {
      // something went wrong, we reset ourselves.
      Serial.end();
      delay(2000);
      printDisplayMask();
      Serial.begin(9600);
      break;
    }
    if (SDP_STATUS_LIVE != newPacket->status) {
      printDisplayMask();
      continue;
    }
    printDisplayFields();
    struct SimDisplayPacket *tmp = oldPacket;
    oldPacket = newPacket;
    newPacket = tmp;
    
    time = micros() - time;

    if (time > 20000) {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}
