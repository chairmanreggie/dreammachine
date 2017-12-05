/*
DREAM MACHINE ALARM CLOCK
Display Code - runs a 7 segment display using multiple sliders as alarm inputs
Latest edit: 12/5/17
Photon ID: 3d003a001051353338363333
Author: Reggie Raye
*/

// ======================== LIBS ==============================

#include <Wire.h>
#include "application.h"
#include "DS1307.h"
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR      false

Adafruit_7segment clockDisplay = Adafruit_7segment();
Adafruit_7segment matrix = Adafruit_7segment();
#define DS1307_ADDRESS 0x68

RTC_DS1307 rtc;

// ======================== VARS ==============================

int hours = 0;
int minutes = 0;
int seconds = 0;
int earliestAlarmHours = 0;       //saves slider 1 value in early alarm hours
int earliestAlarmMinutes = 0;     //saves slider 2 value in early alarm minutes
int latestAlarmHours = 0;
int latestAlarmMinutes = 0;

char sleepQuality = 0;

bool blinkColon = false;

int slider1 = A0; // earliest hour
int slider2 = A1; // earliest minute
int slider3 = A2;
int slider4 = A3;

#define slider1 A0
#define slider2 A1
#define slider3 A2
#define slider4 A3
#define sliderAlarm A4 // on / off
#define inSnooze A5
#define ampmEarly D3        //am or pm selection alarms
#define ampmLate D2

int valSlider1 = 0, pastValSlider1 = 0;      //value slider and past value slider variables
int valSlider2 = 0, pastValSlider2 = 0;
int valSlider3 = 0, pastValSlider3 = 0;
int valSlider4 = 0, pastValSlider4 = 0;
int valSlider5 = 0, pastValSlider5 = 0;

int digit0 = 0;
int digit1 = 0;
int digit3 = 0;
int digit4 = 0;

bool snooze = 0;                // snooze flag
bool AMorPMearly = 0;           // if = 1 then am if = 0 then pm
bool AMorPMlate = 0;
bool AM = 0;
bool PM = 0;

int currentTime = 0; // current display time in XXXX format i.e. 1234 = 12:34
int optimalWakeTime = 0;
int earliestWakeupTime = 0;
int latestWakeupTime = 0;

int AAvgSong = 0, avgSong = 0, BAvgSong = 0;
int sleepDuration = 0, sleepCycles = 0;
int avgAccelMovement = 0;
bool awakeOrREMWillLikelyOccurBeforeLatestWakeup = 0;
bool awakeOrREMHaveNotOccuredBeforeLatestWakeup = 0;

bool NREM3 = 0;
bool NREM2 = 0;
bool NREM1 = 0;
bool REM = 0;
bool awake = 0;

struct sleepStages {
  bool awake = 0;
  bool REM = 0;
  bool NREM1 = 0;
  bool NREM2 = 0;
  bool NREM3 = 0;
};
sleepStages currentSleepStage;

# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

// ======================== SETUP ==============================

void setup()
{
  pinMode(ampmEarly, INPUT);
  pinMode(ampmLate, INPUT);
  Serial.begin(9600);
  matrix.begin(0x70);
  matrix.setBrightness(15);
  Serial1.begin(9600);

  Time.zone(-4);
  execute_CMD(0x3F, 0, 0);

  delay(3000); // wait for console opening

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  /*if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }*/

  // Setup function runs once at startup to initialize the display
  // and DS3231 clock.

  // Setup Serial port to print debug output.
  Serial.begin(115200);
  Serial.println("Clock starting!");
  rtc.adjust(DateTime(Time.local()));
  // Setup the display.
  clockDisplay.begin(DS1307_ADDRESS);

  rtc.begin();

} // end setup

// ======================== LOOP ==============================

void loop() {

  displayTime();
  //readSlidersEtc();
  
  // Serial.print("valSlider1: ");
  // Serial.print(valSlider1);
  // Serial.println();
  // Serial.print("valSlider2: ");
  // Serial.print(valSlider2);
  // Serial.println();
  // Serial.print("valSlider3: ");
  // Serial.print(valSlider3);
  // Serial.println();
  // Serial.print("valSlider4: ");
  // Serial.print(valSlider4);
  // Serial.println();
  // Serial.print("valSlider5: ");
  // Serial.print(valSlider5);
  // Serial.println();
  // delay(100);

  if (pastValSlider1 != valSlider1 || pastValSlider2 != valSlider2) {
    showEarliestAlarm();
    Serial.println("running early alarm if");
  }
  else if (pastValSlider3 != valSlider3 || pastValSlider4 != valSlider4) {
    showLatestAlarm();
    Serial.println("running late alarm if");
  }
  if (valSlider5 == 0) { // while alarm is on do
    Serial.println("alarm is set to " + valSlider5);
    runAlarm();
//    Serial.println("running runAlarm if");
  }
  else if (valSlider5 != 1 && (pastValSlider5 != valSlider5)) { // if alarm slider is turned off
    endAlarm();
//    Serial.println("running endAlarm if");
  }

  updateSliders();

} //end loop

// ======================== ADD'L FUNCTIONS ============================

void displayTime()
{
  DateTime now = rtc.now();
  int hour = now.hour();
  Serial.println("hour = " + hour);

  int minute = now.minute();
  Serial.print("minute = " + minute);

if (hour == 0) { // daylight savings adjustment 1 of 2
  hour = 11;
  digit0 = 1;
  digit1 = 1;
}

else { // daylight savings adjustment 2 of 2
  hour = hour - 1;

  if (hour < 10)
  {
  digit0 = 0;
  digit1 = hour;
  }
  else if (hour == 10 || hour == 11 || hour == 12)
  {
  digit0 = 1;
  digit1 = hour % 10;
  }
  else if (hour > 12)
  {
  digit0 = 0;
  digit1 = hour - 12;
  }

  if (minute > 9) {
  digit3 = minute/10;
  digit4 = minute % 10;
  }
  else {
  digit3 = 0;
  digit4 = minute;
  }

  if (hour >= 12)
  {
  PM = 1;
  AM = 0;
  }
  else if (hour < 12)
  {
  AM = 1;
  PM = 0;
  }
} // end else

  // Serial.print("displayTime running: ");
  // Serial.print(digit0);
  // Serial.print(digit1);
  // Serial.print(":");
  // Serial.print(digit3);
  // Serial.print(digit4);
  // Serial.println();

  if (digit0 == 1)
  {
  matrix.writeDigitNum(0,digit0);
  }
  else if (digit0 != 1)
  {
  matrix.clear();
  }
  matrix.writeDigitNum(1,digit1);
  matrix.drawColon(true);
  matrix.writeDigitNum(3,digit3);
  matrix.writeDigitNum(4,digit4);
  matrix.writeDisplay();

  delay(250);

} //end displayTime


void readSlidersEtc() {

//========= SLIDER1 (earliest hour) ========

  valSlider1 = analogRead(slider1);
  valSlider1 = map(valSlider1, 0, 4096, 1 , 13);
  // Serial.print("valSider1 = ");
  // Serial.print(valSlider1);
  // Serial.println();

//========= SLIDER2 (earliest minute) ========

  valSlider2 = analogRead(slider2);
  valSlider2 = 5*map(valSlider2, 0, 4096, 0, 12);
  // Serial.print("valSider2 = ");
  // Serial.print(valSlider2);
  // Serial.println();

//========= SLIDER3 (latest hour) ========

  valSlider3 = analogRead(slider3);
  valSlider3 = map(valSlider3, 0, 4096, 1, 13);

//========= SLIDER4 (latest minute) ========

  valSlider4 = analogRead(slider4);
  valSlider4 = 5*map(valSlider4, 0, 4096, 0, 12);

//========= SLIDER5 (alarm on/off) ========

  valSlider5 = analogRead(sliderAlarm);
  valSlider5 = map(valSlider5, 0, 4096, 0, 2);

//========= SWITCHES ========
  if (digitalRead(ampmEarly) == HIGH)
  {
    AMorPMearly = 1; // if 1, then am
  //  Serial.println("AMorPMearly = " + AMorPMearly);
  }
  else if (digitalRead(ampmEarly) == LOW)
  {
    AMorPMearly = 0; // if 0, then pm
  //  Serial.println("AMorPMearly = " + AMorPMearly);
  }
  if (digitalRead(ampmLate) == HIGH)
  {
    AMorPMlate = 1;
//    Serial.println("AMorPMlate = " + AMorPMlate);
  }
  else if (digitalRead(ampmLate) == LOW)
  {
    AMorPMlate = 0;
//    Serial.println("AMorPMlate = " + AMorPMlate);
  }

//========= SNOOZE BUTTON ========
    if (digitalRead(inSnooze) == HIGH)
    {
      snooze = 1;
      execute_CMD(0x0E,0,0);
    }
    else if (digitalRead(inSnooze) == LOW)
    {
      snooze = 0;
    }

//========= POPULATING VARS ========
  earliestAlarmHours = valSlider1;
  earliestAlarmMinutes = valSlider2;
  latestAlarmHours = valSlider3;
  latestAlarmMinutes = valSlider4;

  earliestWakeupTime = earliestAlarmHours*100 + earliestAlarmMinutes;
  latestWakeupTime = latestAlarmHours*100 + latestAlarmMinutes;

} // end readSlidersEtc


void showEarliestAlarm() {

  char counterSec = 0; //use char so it can save until value 255
  do
  {
    if (valSlider1 < 10)
    {
     digit0 = 0;
     digit1 = valSlider1 % 10;
    }
    else if (valSlider1 >= 10)
    {
     digit0 = valSlider1/10;
     digit1 = valSlider1 % 10;
    }

    if (valSlider2 < 10)
    {
     digit3 = 0;
     digit4 = valSlider4;
    }
    else if (valSlider2 >= 10)
    {
     digit3 = valSlider2/10;
     digit4 = valSlider2 % 10;
    }

    if (digit0 == 1)
    {
    matrix.writeDigitNum(0,digit0);
    }
    else if (digit0 != 1)
    {
    matrix.clear();
    }
    matrix.writeDigitNum(1,digit1);
    matrix.drawColon(true);
    matrix.writeDigitNum(3,digit3);
    matrix.writeDigitNum(4,digit4);
    matrix.writeDisplay();
    readSlidersEtc();
    if (pastValSlider1 != valSlider1 || pastValSlider2 != valSlider2)
    {
      counterSec = 0;
    }
    delay(10);
    counterSec++;
    updateSliders();
  }
  while(counterSec < 240);

} // end showEarliestAlarm


void showLatestAlarm() {

  char counterSec = 0; //use char so it can save until value 255
  do
  {
    if (valSlider3 < 10)
    {
     digit0 = 0;
     digit1 = valSlider3 % 10;
    }
    else if (valSlider3 >= 10)
    {
     digit0 = valSlider3/10;
     digit1 = valSlider3 % 10;
    }

    if (valSlider4 < 10)
    {
     digit3 = 0;
     digit4 = valSlider4;
    }
    else if (valSlider4 >= 10)
    {
     digit3 = valSlider4/10;
     digit4 = valSlider4 % 10;
    }

    if (digit0 == 1)
    {
    matrix.writeDigitNum(0,digit0);
    }
    else if (digit0 != 1)
    {
    matrix.clear();
    }
    matrix.writeDigitNum(1,digit1);
    matrix.drawColon(true);
    matrix.writeDigitNum(3,digit3);
    matrix.writeDigitNum(4,digit4);
    matrix.writeDisplay();
    readSlidersEtc();
    if (pastValSlider3 != valSlider3 || pastValSlider4 != valSlider4)
    {
      counterSec = 0;
    }
    delay(10);
    counterSec++;
    updateSliders();
  }
  while(counterSec < 240);

} // end showLatestAlarm


void updateSliders() {
    //Serial.println("updating sliders");
    pastValSlider1 = valSlider1;
    pastValSlider2 = valSlider2;
    pastValSlider3 = valSlider3;
    pastValSlider4 = valSlider4;
    pastValSlider5 = valSlider5;
}


void runAlarm() {

//  Serial.println("running runAlarm ");

  currentTime = (digit0 + digit1)*100 + digit3 + digit4;
  detOptimalWakeTime();
  if (snooze==0 && valSlider5==1 && currentTime==optimalWakeTime)
  {
    selectSong();
  }
  else if (snooze==1 && valSlider5==1)
  {
    optimalWakeTime = optimalWakeTime + 1;
    snooze = 0;
  }

    int eeprom_address = 10; //this should be located elsewhere, right?
    struct saveWakeupTimes {
      uint8_t valSlider1;
      uint8_t valSlider2;
      uint8_t valSlider3;
      uint8_t valSlider4;
    };
    saveWakeupTimes wakeupTimes;

    updateSliders();
} // end runAlarm


void endAlarm() {
  execute_CMD(0x0E,0,0);
  updateSliders();
}


void detOptimalWakeTime() {
    optimalWakeTime = earliestWakeupTime;
}


void selectSong() {
    playAAvgSong();
    AAvgSong++;
}


void playAAvgSong() {

  Serial.print("running runAlarm ");
  Serial.println();
  switch(AAvgSong)
  {
    case 1:
    execute_CMD(0x03,0,1);
    break;
    case 2:
    execute_CMD(0x03,0,2);
    break;
    case 3:
    execute_CMD(0x03,0,3);
    break;
    case 4:
    execute_CMD(0x03,0,4);
    break;
    AAvgSong = 0;
    break;
  }
} // end playAAvgSong

void execute_CMD(byte CMD, byte Par1, byte Par2) { // Excecute the command and parameters
 // Calculate the checksum (2 bytes)
 int16_t checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);

 // Build the command line
 byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge, Par1, Par2, checksum >> 8, checksum & 0xFF, End_Byte};

 //Send the command line to the module
 for (byte k=0; k<10; k++)
 {
  Serial1.write( Command_line[k]);
 }
} // end execute_CMD
