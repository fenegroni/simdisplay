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

#include <inttypes.h>

#include <LiquidCrystal.h>

#include "SimDisplayProtocol.h"

// ----------------------  0123456789012345
static char DISPLAY_MASK_ROW0[] = "-   --.-%   -/- ";
static char DISPLAY_MASK_ROW1[] = "R--  M-   --/--C";

#define DISPLAY_ABS_COLROW      0,  0
#define DISPLAY_REMLAPS_COLROW  1,  1
#define DISPLAY_TC_COLROW       11, 0
#define DISPLAY_TCC_COLROW      14, 0
#define DISPLAY_BB_COLROW       4,  0
#define DISPLAY_MAP_COLROW      6,  1
#define DISPLAY_AIRT_COLROW     10, 1
#define DISPLAY_ROADT_COLROW    13, 1

enum {
  RL_CLOCK_PIN = 2,
  RL_LATCH_PIN = 3,
  RL_DATA_PIN = 4,
  RL_CLEAR_PIN = 5
};

enum {
  LCD_RS_PIN = 10,
  LCD_ENABLE_PIN = 11,
  LCD_D4_PIN = 14, // A0
  LCD_D5_PIN = 15, // A1
  LCD_D6_PIN = 16, // A2
  LCD_D7_PIN = 17  // A3
};

/*
  The LiquidCrystal library works with all LCD displays that are compatible with the
  Hitachi HD44780 driver. There are many of them out there, and you
  can usually tell them by the 16-pin interface.

  The circuit:
  LCD RS pin to digital pin 10
  LCD Enable pin to digital pin 11
  LCD D4 pin to digital pin 14 (A0)
  LCD D5 pin to digital pin 15 (A1)
  LCD D6 pin to digital pin 16 (A2)
  LCD D7 pin to digital pin 17 (A3)
  LCD R/W pin to ground
  LCD VSS pin to ground
  LCD VDD pin to 5V
  10K resistor:
  ends to +5V and ground
  wiper to LCD VO pin (pin 3)
*/
LiquidCrystal lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

static struct SimDisplayPacket packet[2];
static struct SimDisplayPacket *newPacket = &packet[0];
static struct SimDisplayPacket *oldPacket = &packet[1];

static void lcdPrint(char *str, int col, int row)
{
  lcd.setCursor(col, row);
  lcd.print(str);
}

static void printDisplayMask()
{
  lcdPrint(DISPLAY_MASK_ROW0, 0, 0);
  lcdPrint(DISPLAY_MASK_ROW1, 0, 1);
}

static void printDisplayField(int newval, int oldval, const char *zerostr, const char *fmtstr, int col, int row)
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
      sprintf(strbuffer, "%d.%d", newval / 10, newval % 10);
    }
    lcdPrint(strbuffer, col, row);
  }
}

static void printDisplayFields()
{
  printDisplayField(newPacket->abs, oldPacket->abs, "- ", "%-2d", DISPLAY_ABS_COLROW);
  printDisplayField(newPacket->remlaps, oldPacket->remlaps, "--", "%2d", DISPLAY_REMLAPS_COLROW);
  printDisplayField(newPacket->tc, oldPacket->tc, " -", "%2d", DISPLAY_TC_COLROW);
  printDisplayField(newPacket->tcc, oldPacket->tcc, "- ", "%-2d", DISPLAY_TCC_COLROW);
  printDisplayFieldBB(newPacket->bb, oldPacket->bb, DISPLAY_BB_COLROW);
  printDisplayField(newPacket->map, oldPacket->map, "- ", "%-2d", DISPLAY_MAP_COLROW);
  printDisplayField(newPacket->airt, oldPacket->airt, "--", "%2d", DISPLAY_AIRT_COLROW);
  printDisplayField(newPacket->roadt, oldPacket->roadt, "--", "%2d", DISPLAY_ROADT_COLROW);
}

void writeRedline(uint8_t pattern)
{
  digitalWrite(RL_LATCH_PIN, LOW);
  shiftOut(RL_DATA_PIN, RL_CLOCK_PIN, LSBFIRST, pattern);
  digitalWrite(RL_LATCH_PIN, HIGH);
}

static void clearRedline()
{
  digitalWrite(RL_LATCH_PIN, LOW);
  digitalWrite(RL_CLEAR_PIN, LOW);
  digitalWrite(RL_LATCH_PIN, HIGH);
  digitalWrite(RL_CLEAR_PIN, HIGH);
}

static void printRedline()
{
  static unsigned long bktm = 0;
  static uint8_t bksta = B00000000;
  static const unsigned long bkint = 100;

  if (newPacket->rpm > newPacket->shftrpm) {
    if (bktm > 0 && millis() - bktm < bkint) {
      return;
    }
    writeRedline(bksta = ~bksta);
    bktm = millis();
    return;
  }
  
  bktm = 0;
  bksta = B00000000;
  
  if (newPacket->rpm > newPacket->optrpm) {
    static const int RLSTAGES = 8;
    static uint8_t patterns[RLSTAGES] = {
      B00000001,
      B00000011,
      B00000111,
      B00001111,
      B00011111,
      B00111111,
      B01111111,
      B11111111,
    };
  
    int stage = (newPacket->rpm - newPacket->optrpm) / ((newPacket->shftrpm - newPacket->optrpm) / RLSTAGES);
    writeRedline(patterns[stage]);
    return;
  }
  
  clearRedline();
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  lcd.begin(16, 2);
  printDisplayMask();

  pinMode(RL_LATCH_PIN, OUTPUT);
  pinMode(RL_CLOCK_PIN, OUTPUT);
  pinMode(RL_DATA_PIN, OUTPUT);
  pinMode(RL_CLEAR_PIN, OUTPUT);
  clearRedline();

  Serial.begin(9600);
}

void loop()
{
  static const int packetsize = sizeof(struct SimDisplayPacket);
  while (Serial.available() >= packetsize) {
    unsigned long time = micros();

    if (packetsize > Serial.readBytes((byte *)newPacket, packetsize)) {
      // something went wrong, we reset ourselves.
      Serial.end();
      delay(2000);
      printDisplayMask();
      clearRedline();
      Serial.begin(9600);
      break;
    }
    if (SDP_STATUS_LIVE != newPacket->status && SDP_STATUS_PAUSE != newPacket->status) {
      printDisplayMask();
      clearRedline();
      continue;
    }
    printRedline();
    printDisplayFields();
    struct SimDisplayPacket *tmp = oldPacket;
    oldPacket = newPacket;
    newPacket = tmp;

    time = micros() - time;

    if (time > 20000) {
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}
