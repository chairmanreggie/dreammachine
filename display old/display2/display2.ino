/***************************************************
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED 7-Segment backpacks
  ----> http://www.adafruit.com/products/881
  ----> http://www.adafruit.com/products/880
  ----> http://www.adafruit.com/products/879
  ----> http://www.adafruit.com/products/878

  These displays use I2C to communicate, 2 pins are required to
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

unsigned long thyme;


void setup() {
//#ifndef __AVR_ATtiny85__
  Serial.begin(9600);
  Serial.println("hello...");
//#endif
  matrix.begin(0x70);
  matrix.setBrightness(15);
  Time.zone(-4);
}

void loop() {

  // try to print a number thats too long
  //matrix.print(10000, DEC);
  //matrix.writeDisplay();
  //delay(500);

  //  Serial.println("7 Segment Backpack Test");
  //Serial.println(Time.now());

  /*Serial.print("Time: ");
    thyme = millis();
    //prints time since program started
    Serial.println(thyme);
    // wait a second so as not to send massive amounts of data
    delay(1000);*/

  //print(0666);

  Serial.println("made it to the start");
  Serial.print(Time.hourFormat12());
  Serial.print(":");
  Serial.print(Time.minute());
  Serial.print(":");
  Serial.print(Time.second());
  Serial.println();
  delay(1000);
//  matrix.print(1234,DEC);
  //matrix.writeDigitRaw(2, 0x02);

  matrix.drawColon(true);
  int hour = Time.hourFormat12();
  int minute = Time.minute();
  int digit0 = hour/10;
  int digit1 = hour % 10;
  int digit3 = minute/10;
  int digit4 = minute % 10;
  if (digit0 != 0) {
  matrix.writeDigitNum(0,digit0);
  }
  matrix.writeDigitNum(1,digit1);
  matrix.writeDigitNum(3,digit3);
  matrix.writeDigitNum(4,digit4);
  matrix.writeDisplay();
  delay(1000);

Serial.println("made it to the end");
  /*
  matrix.print(0666,DEC);
  matrix.writeDisplay();
  delay(1000);
matrix.print(6000,DEC);
  matrix.writeDisplay();
  delay(1000);

  matrix.writeDigitNum(0, 22);
  matrix.writeDisplay();
  delay(500);
  matrix.writeDigitNum(4, 33);
  matrix.writeDisplay();
  delay(500);

  // print a hex number
  matrix.print(0xBEEF, HEX);
  matrix.writeDisplay();
  delay(1000);

  // print a floating point
  matrix.print(12.34);
  matrix.writeDisplay();
  delay(500);

  // print with print/println
  for (uint16_t counter = 0; counter < 9999; counter++) {
    matrix.println(counter);
    matrix.writeDisplay();
    delay(10);
  }

  // method #2 - draw each digit
  uint16_t blinkcounter = 0;
  boolean drawDots = false;
  for (uint16_t counter = 0; counter < 9999; counter ++) {
    matrix.writeDigitNum(0, (counter / 1000), drawDots);
    matrix.writeDigitNum(1, (counter / 100) % 10, drawDots);
    matrix.drawColon(drawDots);
    matrix.writeDigitNum(3, (counter / 10) % 10, drawDots);
    matrix.writeDigitNum(4, counter % 10, drawDots);

    blinkcounter+=50;
    if (blinkcounter < 500) {
      drawDots = false;
    } else if (blinkcounter < 1000) {
      drawDots = true;
    } else {
      blinkcounter = 0;
    }
    matrix.writeDisplay();
    delay(10);
  }*/
} //end loop
