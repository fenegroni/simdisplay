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
FIXME: in feature/show-rpm-redline the pins are configured differently.

LED Pins 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9
These represent an RPM redline, whereby LEDs are either fully on or fully off.
The first LED is lit after an optimal upshift.
The LEDs are lit up in sequence, with equal percentage, to show the RPM
value approaching the Optimal shift point.
When the Optimal shift point is reached, the last LED is lit.
Past that, the LEDs will blink to indicate we are close to the maxrpm.

The pin layout will be merged once work in this branch is complete.
We will use 6 pins: 3 for the redline rpm LEDs
and 3 for the gear indicator segment LEDs.
If necessary there are analog input pins that can be configured
as digital output to be used for damage, TC active and ABS active LEDs.

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
#define DISPLAY_MASK_ROW0 "-   --.-%   -/- "
#define DISPLAY_MASK_ROW1 "R--  M-   --/--C"

#define DISPLAY_ABS_COLROW      0,  0
#define DISPLAY_REMLAPS_COLROW  1,  1
#define DISPLAY_TC_COLROW       11, 0
#define DISPLAY_TCC_COLROW      14, 0
#define DISPLAY_BB_COLROW       4,  0
#define DISPLAY_MAP_COLROW      6,  1
#define DISPLAY_AIRT_COLROW     10, 1
#define DISPLAY_ROADT_COLROW    13, 1

enum {
  RLLEDPIN_1 = 2,
  RLLEDPIN_2 = 3,
  RLLEDPIN_3 = 4,
  RLLEDPIN_4 = 5,
  RLLEDPIN_5 = 6,
  RLLEDPIN_6 = 7,
  RLLEDPIN_7 = 8,
  RLLEDPIN_8 = 9
};

// We use the display's 4 bit interface during development.
// If we have more pins available we can make it faster
// by switching to the 8 bit interface.
//LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // TODO: make it clearer how we are initialising the library.

static struct SimDisplayPacket packet[2];
static struct SimDisplayPacket *newPacket = &packet[0];
static struct SimDisplayPacket *oldPacket = &packet[1];

static void lcdPrint(char *str, int col, int row)
{
  //lcd.setCursor(col, row);
  //lcd.print(str);
}

static void printDisplayMask()
{
  lcdPrint(DISPLAY_MASK_ROW0, 0, 0);
  lcdPrint(DISPLAY_MASK_ROW1, 0, 1);
}

static void printDisplayField(int newval, int oldval, char *zerostr, char *fmtstr, int col, int row)
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

static void printDisplayFieldBB(int newval, int oldval, int col, int row)
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

static void printDisplayFields()
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

static int rlledpins[] = {RLLEDPIN_1, RLLEDPIN_2, RLLEDPIN_3, RLLEDPIN_4, RLLEDPIN_5, RLLEDPIN_6, RLLEDPIN_7, RLLEDPIN_8 };

static void printRedline()
{
  /*
    1 optrpm
    2 optrpm + 1 * step
    3 optrpm + 2 * step
    4 optrpm + 3 * step
    5 optrpm + 4 * step
    6 optrpm + 5 * step
    7 optrpm + 6 * step
    8 shftrpm (7 steps)
   */
  int step = (newPacket->shftrpm - newPacket->optrpm) / 7; // 7250 - 5600 = 1650; 1650 / 7 = 235

  // switch leds on and off in one loop!
  for (int led = 0, ledrpm = newPacket->optrpm; led < 8; ++led, ledrpm += step) {
    if (newPacket->rpm >= ledrpm) {
      digitalWrite(rlledpins[led], HIGH);
    } else {
      digitalWrite(rlledpins[led], LOW);
    }
  }

}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  //lcd.begin(16, 2); FIXME reinstate
  //printDisplayMask(); FIXME reinstate

  for (int led = 0; led < 8; ++led) {
    pinMode(rlledpins[led], OUTPUT);
  }
  
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
      //printDisplayMask(); FIXME reinstate
      Serial.begin(9600);
      break;
    }
    if (SDP_STATUS_LIVE != newPacket->status && SDP_STATUS_PAUSE != newPacket->status) {
      //printDisplayMask();
      continue;
    }
    // Insert code to light up RPM here
    printRedline();
    //printDisplayFields(); FIXME reinstate
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
