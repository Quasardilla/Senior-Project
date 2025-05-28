/*Copyright (c) 2010 bildr community

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "mpr121.h"
#include <Wire.h>

int M_PIN = 5;  //Motor Pin

int irqpin = 2;           // Digital 2
boolean touchStates[12];  //to keep track of the previous touch states
boolean touchPad[5][5];   //to keep track of the previous touch states

boolean isPressed = false;
boolean isScrolling = false;

int startPressing = 0;
int stopPressing = 0;

float ppX = 0;  //previous PREVIOUS x
float ppY = 0;
float pX = 0;  //previous x
float pY = 0;
float X = NAN;  // x -- NaN so it doesn't start clicked
float Y = NAN;

float vX;  // change between x and pX
float vY;
float pvX;  // change between pX and ppX
float pvY;

float aX;  // change between vx and pvx
float aY;

int count = 0;

void setup() {
  pinMode(irqpin, INPUT);
  digitalWrite(irqpin, HIGH);  //enable pullup resistor

  Serial.begin(9600);
  Wire.begin();

  mpr121_setup();

  playStartupSound();
}

void loop() {
  setPreviousPositions();

  readTouchInputs();

  calcVelocities();

  calcAccelerations();

  count++;

  if (!std::isnan(X) || !std::isnan(Y)) {
    Serial.print("X:");
    Serial.print(X, 2);
    Serial.print(" Y:");
    Serial.print(Y, 2);
    Serial.print(" vX:");
    Serial.print(vX, 2);
    Serial.print(" vY:");
    Serial.print(vY, 2);
    Serial.print(" aX:");
    Serial.print(aX, 2);
    Serial.print(" aY:");
    Serial.println(aY, 2);

    if (!isPressed && !isScrolling) {
      startPressing = millis();
      isPressed = true;
      clickPress();
    }

    if (millis() - startPressing > 100 && (vX != 0 && !std::isnan(vX)) || (vY != 0 && !std::isnan(vY))) {
      isScrolling = true;
      isPressed = false;
    }
 
  } else {
    if (isPressed) {
      isPressed = false;
      stopPressing = millis();

      if (stopPressing - startPressing > 50 && stopPressing - startPressing < 1000) {
        clickRelease();
      }

      startPressing = 0;
      stopPressing = 0;
    } else if (isScrolling) {
      isScrolling = false;
    }
  }

  if (isScrolling && X > 0 && X < 4 && Y > 0 && Y < 4 && ((vX != 0 && !std::isnan(vX)) || (vY != 0 && !std::isnan(vY)))) {
    scrollClick();
  }

  if(isScrolling)
    delay(0); // Allows the touchpad to be reactive of MUCH higher speeds -- 2.85x faster than normal (there's a 700 microSecond delay from each scroll tick)
  else
    delay(2);
}


void readTouchInputs() {
  if (!checkInterrupt()) {

    //read the touch state from the MPR121
    Wire.requestFrom(0x5A, 2);

    byte LSB = Wire.read();
    byte MSB = Wire.read();

    uint16_t touched = ((MSB << 8) | LSB);  //16bits that make up the touch states

    for (int i = 0; i < 12; i++) {  // Check what electrodes were pressed
      if (touched & (1 << i)) {
        touchStates[i] = 1;
      } else {
        touchStates[i] = 0;
      }
    }

    X = calculateAverage(0, touchStates);
    Y = calculateAverage(5, touchStates);
  }
}

float calculateAverage(int offset, boolean* arr) {
  int total = 0;
  int count = 0;

  for (int i = 0 + offset; i < 5 + offset; i++) {
    if (arr[i]) {
      count++;
      total += (i - offset);
    }
  }

  return (float)total / (float)count;
}


void mpr121_setup(void) {

  set_register(0x5A, ELE_CFG, 0x00);

  // Section A - Controls filtering when data is > baseline.
  set_register(0x5A, MHD_R, 0x01);
  set_register(0x5A, NHD_R, 0x01);
  set_register(0x5A, NCL_R, 0x00);
  set_register(0x5A, FDL_R, 0x00);

  // Section B - Controls filtering when data is < baseline.
  set_register(0x5A, MHD_F, 0x01);
  set_register(0x5A, NHD_F, 0x01);
  set_register(0x5A, NCL_F, 0xFF);
  set_register(0x5A, FDL_F, 0x02);

  // Section C - Sets touch and release thresholds for each electrode
  set_register(0x5A, ELE0_T, TOU_THRESH);
  set_register(0x5A, ELE0_R, REL_THRESH);

  set_register(0x5A, ELE1_T, TOU_THRESH);
  set_register(0x5A, ELE1_R, REL_THRESH);

  set_register(0x5A, ELE2_T, TOU_THRESH);
  set_register(0x5A, ELE2_R, REL_THRESH);

  set_register(0x5A, ELE3_T, TOU_THRESH);
  set_register(0x5A, ELE3_R, REL_THRESH);

  set_register(0x5A, ELE4_T, TOU_THRESH);
  set_register(0x5A, ELE4_R, REL_THRESH);

  set_register(0x5A, ELE5_T, TOU_THRESH);
  set_register(0x5A, ELE5_R, REL_THRESH);

  set_register(0x5A, ELE6_T, TOU_THRESH);
  set_register(0x5A, ELE6_R, REL_THRESH);

  set_register(0x5A, ELE7_T, TOU_THRESH);
  set_register(0x5A, ELE7_R, REL_THRESH);

  set_register(0x5A, ELE8_T, TOU_THRESH);
  set_register(0x5A, ELE8_R, REL_THRESH);

  set_register(0x5A, ELE9_T, TOU_THRESH);
  set_register(0x5A, ELE9_R, REL_THRESH);

  set_register(0x5A, ELE10_T, TOU_THRESH);
  set_register(0x5A, ELE10_R, REL_THRESH);

  set_register(0x5A, ELE11_T, TOU_THRESH);
  set_register(0x5A, ELE11_R, REL_THRESH);

  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(0x5A, FIL_CFG, 0x04);

  // Section E
  // Electrode Configuration
  // Set ELE_CFG to 0x00 to return to standby mode
  set_register(0x5A, ELE_CFG, 0x0C);  // Enables all 12 Electrodes


  // Section F
  // Enable Auto Config and auto Reconfig
  /*set_register(0x5A, ATO_CFG0, 0x0B);
  set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5A, ATO_CFGT, 0xB5);*/
  // Target = 0.9*USL = 0xB5 @3.3V

  set_register(0x5A, ELE_CFG, 0x0C);
}


boolean checkInterrupt(void) {
  return digitalRead(irqpin);
}


void set_register(int address, unsigned char r, unsigned char v) {
  Wire.beginTransmission(address);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}

void pulseMotor(int val, int millis) {
  if (val > 155) {
    Serial.println("----- INVALID MOTOR VAL -----");
    return;
  }

  analogWrite(M_PIN, val);  // 155 is the MAXIMUM for the voice coil
  delay(millis);
  analogWrite(M_PIN, 0);  // 155 is the MAXIMUM for the voice coil
  delay(millis);
}

void playStartupSound() {
  for (int i = 0; i < 12; i++)
    pulseMotor(10, 6);
  for (int i = 0; i < 20; i++)
    pulseMotor(15, 4);
  for (int i = 0; i < 40; i++)
    pulseMotor(20, 2);
}

void scrollClick() {
  analogWrite(M_PIN, -1);  // 155 is the MAXIMUM for the voice coil
  delayMicroseconds(400);
  analogWrite(M_PIN, 0);  // 155 is the MAXIMUM for the voice coil
  Serial.println("----- CLICK CLICK -----");
}

void clickPress() {
  analogWrite(M_PIN, -1);  // 155 is the MAXIMUM for the voice coil
  delayMicroseconds(1200);
  analogWrite(M_PIN, 0);  //
}

void clickRelease() {
  Serial.println("CLICK");

  pulseMotor(15, 8);
  pulseMotor(-155, 4);
}

void setPreviousPositions() {
  ppX = pX;
  ppY = pY;
  pX = X;
  pY = Y;
}

void calcVelocities() {

  if(!std::isnan(X) && !std::isnan(pX))
    vX = X - pX;

  if(!std::isnan(Y) && !std::isnan(pY))
    vY = Y - pY;

  if(!std::isnan(pX) && !std::isnan(ppX))
    pvX = pX - ppX;

  if(!std::isnan(pY) && !std::isnan(ppY))
    pvY = pY - ppY;
}

void calcAccelerations() {
  if(!std::isnan(vX) && !std::isnan(pvX))
    aX = vX - pvX;
  
  if(!std::isnan(vY) && !std::isnan(pvY))
    aY = vY - pvY;
}
