/*
  SimpleCounter - Simple 2 digit Counter, battery powered.

  Created by: Rafal Tomczak, 2020
              rafal.tomczak@m2p.pl

    up / down counter (2 buttons) with common cathode 2 digit segment display
    Simple to extreme: 
        * arduino mini pro, 
        * 2 digit display (with long pins), 
        * resistor 100 Ohm (optional? added for safety and easy assembly)
        * 2 buttons with connectors (or single 2 state monostable button)
        * battery holder 3 x AAA with On/Off switch,

    Not needed: board
    Extra: 3d printed case
    
  Compile options:
    Arduino 1.8.9 (verified)
    board:
    Arduino Pro or Pro Mini
    Atmega 328P (3.3V 8MHz)
    
  Needed libraries:
    AceButton (button debouncing and more) https://github.com/bxparks/AceButton
    SevSeg    (segment display handling)   https://github.com/DeanIsMe/SevSeg
    Arduino_Vcc (battery voltage)          https://github.com/Yveaux/arduino_vcc 
       (library not standard arduino, so included a copy in project, changed battery range clipping to 99%)


  Description:
    Simple device for couting up to 99 by pressing one of two buttons (+/Up) (-/Down).
    Use case: Counting people in shop. When someone enter - push UP, when someone leaving - push DOWN
              Should be very usefull during Corona Virus empidemic restrictions.

  Extra functions:
      - hold UP button - shows remaining battery % (need to be calibrated)
      - hold DOWN button - show how many times (+/Up) was pressed since power ON
          cycle by every digit, with extra marking to define which digitit is.
          it looks complicated, but after a while it should be easy to read, 
            example: 123 will show
              1  (left dot)
              2  (right dot)
              3  (no dots)
              (pause)
             example 2: 12,345 will show
              1 _ (right dot)
              2 _ (no dots)
              (pause)
              3  (left dot)
              4  (right dot)
              5  (no dots)
              (pause)

  License:
  This software is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this software; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/

//================================================================================================
// CONFIGURATION 
// PINOUT: ---------------------------------------------------------
// uncoment "#define" to cycle for every part of segment display, starts from Left digit diode "A" ... "DP" then Right "A" .. "DP"
// modify segmentPins1 / 2 to map correct diodes.
//#define CHECK_PINOUT
const byte digitPins[] = {6};  // unused pin (not needed, but required by sevseg library)
  /*  segmentPins meaning
   *      -- A -- 
   *     |       |
   *     F       B
   *     |       |
   *      -- G -- 
   *     |       |
   *     E       C
   *     |       |
   *      -- D --  DP
   *  { A, B, C, D, E, F, G, DP}
   */
   // LEFT
   //                           A  B  C  D  E  F  G DP
//const byte segmentPins1[] = {  ,  ,  ,  ,  ,  ,  ,  };
  const byte segmentPins1[] = { 4, 5,A2,A3,A5, 2, 3,A1};
  //                            1  2  3  4  5  6  7  8     
  // RIGHT
  //                            A  B  C  D  E  F  G DP
//const byte segmentPins2[] = {  ,  ,  ,  ,  ,  ,  ,  };
  const byte segmentPins2[] = { 8, 9,11,13,A0, 7,12,10};
  //                            1  2  3  4  5  6  7  8     

// VOLTAGE:  --------------------------------------------------------
// uncomment "#define" to enable long press DOWN to show read voltage to correct calibration
//#define CHECK_VOLTAGE
const float VccCorrection = 1.0/1.0;  // Measured Vcc by multimeter divided by reported Vcc
const float VccMin   = 3.2;           // Minimum expected Vcc level, in Volts. (0% battery)
const float VccMax   = 4.2;           // Maximum expected Vcc level, in Volts. (99% battery)

// Brightnes (-200..200) --------------------------------------------
const int brightnes = 200;

//================================================================================================
// Display

#include "SevSeg.h"
SevSeg sevseg1; //Instantiate a seven segment controller object, 1st digit
SevSeg sevseg2; //Instantiate a seven segment controller object, 2nd digit

// Buttons
#include <AceButton.h>
using namespace ace_button;

const int BUTTON_PIN_UP   = 0;  // RxD
const int BUTTON_PIN_DOWN = 1;  // TxD

ButtonConfig buttonConfigUp;
ButtonConfig buttonConfigDown;
AceButton buttonUp(&buttonConfigUp);
AceButton buttonDown(&buttonConfigDown);

//AceButton buttonUp(BUTTON_PIN_UP);
//AceButton buttonDown(BUTTON_PIN_DOWN);

void handleEventUp(AceButton*, uint8_t, uint8_t);
void handleEventDown(AceButton*, uint8_t, uint8_t);

// Voltage
#include "vcc.h"

Vcc vcc(VccCorrection);


// counter itself
static int counter = 0; // it need to be unsigned !
static long unsigned total = 0;  // total number of Up during power ON

static int current_digit = 0;
static boolean presentation_mode = false;

// the setup function runs once when you press reset or power the board
void setup() {
  byte numDigits = 1;
  bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected

  sevseg1.begin(hardwareConfig, numDigits, digitPins, segmentPins1, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  sevseg1.setBrightness(brightnes);
  sevseg2.begin(hardwareConfig, numDigits, digitPins, segmentPins2, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  sevseg2.setBrightness(brightnes);

  // inputs (keys), button connect to GND
  pinMode(BUTTON_PIN_UP, INPUT_PULLUP);
  pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP);
  
  buttonConfigDown.setEventHandler(handleEventDown);
  buttonConfigUp.setEventHandler(handleEventUp);
  buttonConfigDown.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfigUp.setFeature(ButtonConfig::kFeatureLongPress);
  buttonUp.init(BUTTON_PIN_UP);
  buttonDown.init(BUTTON_PIN_DOWN);

  // init counter, maybe it should be stored and read from EEPROM?
  counter = 0;

  // init display with counter value
  sevseg1.setNumber(counter/10); //0=dot
  sevseg2.setNumber(counter%10, 0);
}

// the loop function runs over and over again forever
void loop() {
  static unsigned long timer = millis();
  static unsigned long tmp=0;

  #ifdef CHECK_PINOUT
    // pinout check only 
    // togle every diode one by one acording definition high digit, low digit, ABC ...
    // left first
    for (int i=0;i<8;i++) {
      digitalWrite(segmentPins1[i], 1);
      delay(1000);
      digitalWrite(segmentPins1[i], 0);
      delay(1000);
    }
    // right second
     for (int i=0;i<8;i++) {
      digitalWrite(segmentPins2[i], 1);
      delay(1000);
      digitalWrite(segmentPins2[i], 0);
      delay(1000);
    }

  #else

  // presentation mode (for long number show)
  if (presentation_mode) {
    static byte txt[21]; // number of digits
     
    if (millis() >= timer) {   
      timer = millis() + 1000; // change every 1 sec
    
      if (current_digit < 0) {
        current_digit = 0;
        // calculate numer of digits
        tmp = total;
        txt[0]=tmp % 10;
        tmp = tmp / 10;
        while (tmp>0 and current_digit<20) {
          current_digit++;
          txt[current_digit] = tmp % 10;
          tmp = tmp / 10;
        }
      }

      // which digit in row(3)   210 210 210

      if ((current_digit % 3) == 2)
        sevseg1.setNumber(txt[current_digit],0); // dot
      else
        sevseg1.setNumber(txt[current_digit] ); // no dot
      // 
      byte r[3];
      r[0]=0;
      r[1]=0;
      switch (current_digit/3) {
        case 6: r[0]=48;break; //
        case 5: r[0]=99;break; //
        case 4: r[0]=92;break; // 
        case 3: r[0]=1;break;  // high   -
        case 2: r[0]=64;break; // middle - 
        case 1: r[0]=8;break;  // low    _
      }
      if ((current_digit % 3) == 1)
        r[0]|=128; // add DOT
      sevseg2.setSegments(r); 

      if ((current_digit % 3) == 0) // wait longer afrer row()
        timer = timer + 2000L;
      current_digit--;
    }
  }


  // Must run repeatedly
  sevseg1.refreshDisplay(); 
  sevseg2.refreshDisplay(); 
  buttonUp.check();
  buttonDown.check();
  
  #endif
}

// ---------------------------------------
// event handlers

void handleEventUp(AceButton* /* button */, uint8_t eventType, uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventPressed:
      total++;
      counter++;
      break;
    case AceButton::kEventReleased:
      if (counter > 99)
        counter=99;
      // update display with new counter value when button released
      sevseg1.setNumber(counter/10);
      sevseg2.setNumber(counter%10, 0);
      break;
    case AceButton::kEventLongPressed:
      total--;   // fix for kEventPressed
      counter--; // fix for kEventPressed
      // get voltage - calibrated in %
      float v = vcc.Read_Perc(VccMin, VccMax,true); // percent battery (calibrated 0 - 99)
      int iV = (int)v;
      sevseg1.setNumber(iV/10);  //0=dot,
      sevseg2.setNumber(iV%10);
  }
}

void handleEventDown(AceButton* /* button */, uint8_t eventType, uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventPressed:
      counter--;
      break;
    case AceButton::kEventReleased:
      // We trigger on the Released event because LongPressed event is triggered after EventPressed.
      if (counter < 0)
        counter=0;    
      // update display with new counter value when button released
      sevseg1.setNumber(counter/10);
      sevseg2.setNumber(counter%10, 0);

      presentation_mode = false; // disable presentation mode 
      break;
    case AceButton::kEventLongPressed:
      counter++; // fix kEventPressed

      #ifdef CHECK_VOLTAGE
        // configure only: get voltage in V --- only for voltage calibration purpouses
        float v = vcc.Read_Volts();
        sevseg1.setNumber(int(v*10)/10, 0);
        sevseg2.setNumber(int(v*10)%10);
      #else
        // standard behaviour, show total number of persons (digit by digit presentation)
        presentation_mode = true;
        current_digit=-1;
      #endif
  }
}


  
