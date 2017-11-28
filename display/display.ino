/*
DREAM MACHINE ALARM CLOCK
Display Code - runs a 7 segment display using multiple sliders as alarm inputs
Latest edit: 11/28/17
*/

// ======================== LIBS ==============================

#include <Wire.h>
#include "application.h"
#include "DS1307.h"
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR      false

// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.

// Create display and DS1307 objects.  These are global variables that
// can be accessed from both the setup and loop function below.
Adafruit_7segment clockDisplay = Adafruit_7segment();
Adafruit_7segment matrix = Adafruit_7segment();
#define DS1307_ADDRESS 0x68

// Date and time functions using a DS1307 RTC connected via I2C
//
// WIRE IT UP!
//
// DS1307               SPARK CORE
//--------------------------------------------------------------------
// VCC                - Vin (5V only, does not work on 3.3)
// Serial Clock (SCL) - D1 (needs 2.2k to 10k pull up resistor to Vin)
// Serial Data  (SDA) - D0 (needs 2.2k to 10k pull up resistor to Vin)
// Ground             - GND
//--------------------------------------------------------------------

RTC_DS1307 rtc;

// ======================== VARS ==============================

int hours = 0;
int minutes = 0;
int seconds = 0;
int earliestAlarmHours = 0;       //saves slider 1 value in early alarm hours
int earliestAlarmMinutes = 0;     //saves slider 2 value in early alarm minutes
int latestAlarmHours = 0;
int latestAlarmMinutes = 0;

char sleepQuality=0;

bool blinkColon = false;

int slider1 = A0;
int slider2 = A1;
int slider3 = A2;
int slider4 = A3;

#define slider1 A0
#define slider2 A1
#define slider3 A2
#define slider4 A3
#define sliderAlarm A4
#define inSnooze A5
#define ampmEarly D3        //am or pm selection alarms
#define ampmLate D4

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
bool awakeOrREMHaveNotOccuredBeforeLatestWakeup == 0;

struct sleepStages {
  bool awake = 0;
  bool REM = 0;
  bool NREM1 = 0;
  bool NREM2 = 0;
  bool NREM3 = 0;
};
sleepStages currentSleepStage;

// ======================== SETUP ==============================

void setup()
{
  pinMode(ampmEarly,INPUT);
  pinMode(ampmLate,INPUT);
  Serial.begin(9600);
  matrix.begin(0x70);
  matrix.setBrightness(15);
  Time.zone(-4);

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

void loop()
{
  displayTime();
  readSlidersEtc();

/*
  Serial.print("valSlider1: ");
  Serial.print(valSlider1);
  Serial.println();
  Serial.print("valSlider2: ");
  Serial.print(valSlider2);
  Serial.println();
  Serial.print("valSlider3: ");
  Serial.print(valSlider3);
  Serial.println();
  Serial.print("valSlider4: ");
  Serial.print(valSlider4);
  Serial.println();
  Serial.print("valSlider5: ");
  Serial.print(valSlider5);
  Serial.println();
  delay(100);*/

  if (pastValSlider1 != valSlider1 || pastValSlider2 != valSlider2)
  {
    showEarliestAlarm();
  }
  else if (pastValSlider3 != valSlider3 || pastValSlider4 != valSlider4)
  {
    showLatestAlarm();
  }
  if (valSlider5 == 1 && (pastValSlider5 != valSlider5)) // if alarm slider is turned on
  {
    runAccel();
  }
  if (valSlider5 == 1) // while alarm is on do
  {
    runAlarm();
  }
  else if (valSlider5 != 1 && (pastValSlider5 != valSlider5)) // if alarm slider is turned off
  {
    endAlarm();
  }

} //end loop

// ======================== ADD'L FUNCTIONS ============================

void displayTime()
{
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();

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

  Serial.print("displayTime running: ");
  Serial.print(digit0);
  Serial.print(digit1);
  Serial.print(":");
  Serial.print(digit3);
  Serial.print(digit4);
  Serial.println();

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

void readSlidersEtc()
{
//========= SLIDER1 (earliest hour) ========

  valSlider1 = analogRead(slider1);
  valSlider1 = map(valSlider1, 0, 4096, 1 , 13);

//========= SLIDER2 (earliest minute) ========

  valSlider2 = analogRead(slider2);
  valSlider2 = 5*map(valSlider2, 0, 4096, 0, 12);

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
  }
  else if (digitalRead(ampmEarly) == LOW)
  {
    AMorPMearly = 0; // if 0, then pm
  }
  if (digitalRead(ampmLate) == HIGH)
  {
    AMorPMlate = 1;
  }
  else if (digitalRead(ampmLate) == LOW)
  {
    AMorPMlate = 0;
  }

//========= SNOOZE BUTTON ========
    if (digitalRead(inSnooze) == HIGH)
    {
      snooze = 1;
    }
    else if (digitalRead(inSnooze) == LOW)
    {
      snooze = 0;
    }

  earliestAlarmHours = valSlider1;
  earliestAlarmMinutes = valSlider2;
  latestAlarmHours = valSlider3;
  latestAlarmMinutes = valSlider4;

  earliestWakeupTime = earliestAlarmHours*100 + earliestAlarmMinutes;
  latestWakeupTime = latestAlarmHours*100 + latestAlarmMinutes;

} // end readSlidersEtc

void showEarliestAlarm()
{
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
    if (pastValSlider3 != valSlider3 || pastValSlider4 != valSlider4)
    {
      counterSec = 0;
    }
    delay(10);
    counterSec++;
    updateSliders();
  }
  while(counterSec < 240);

} // end showEarliestAlarm

void showLatestAlarm()
{
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

void updateSliders()
{
    pastValSlider1 = valSlider1;    //update slider value
    pastValSlider2 = valSlider2;
    pastValSlider3 = valSlider3;
    pastValSlider4 = valSlider4;
    pastValSlider5 = valSlider5;
}

void runAlarm()
{
  currentTime = (digit0 + digit1)*100 + digit3 + digit4;
  detOptimalWakeTime();
  if (snooze==0 && valSlider5==1 && currentTime==optimalWakeTime)
  {
    selectSong();
  }
  else if (snooze==1 && valSlider5==1)
  {
    optimalWakeTime = optimalWakeTime + 10;
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

void endAlarm()
{
//turn off speaker
//turn off runAccel
  optimalWakeTime = 0;
  sleepDuration = 0;
  sleepCycles = 0;
  updateSliders();
}

void detOptimalWakeTime()
{
  detSleepStage();
  if (currentSleepStage.awake == 1 || currentSleepStage.REM == 1)
  {
    optimalWakeTime = earliestWakeupTime;
  }
  else if (awakeOrREMWillLikelyOccurBeforeLatestWakeup == 1) //the idea here is to postpone wakeup if it seems probable that REM or awake will occur prior to the latest wakeup
  {
    if (currentSleepStage.awake == 1 || currentSleepStage.REM == 1) //not sure if these parameters are appropriate, given the purpose of this 'else if'
    {
      optimalWakeTime = currentTime;
    }
  }
else if (awakeOrREMHaveNotOccuredBeforeLatestWakeup == 1)
  {
    optimalWakeTime = latestWakeupTime;
  }

Serial.print("optimalWakeTime: ");
Serial.print(optimalWakeTime);
Serial.println();

} //end detOptimalWakeTime

void detSleepStage()
{
  // get avgAccelMovement from accelerometer.ino (not there now, though that is where it likely belongs)
  // avgAccelMovement might be defined as the weighted average of the absolute values of X (important), Y (important), and Z (less important) accelerometer readings
  if (avgAccelMovement >= 0 && avgAccelMovement <= 1)
  {
    NREM3 = 1;
    NREM2 = 0;
    NREM1 = 0;
    REM = 0;
    awake = 0;
  }
  else if (avgAccelMovement > 1 && avgAccelMovement <= 2)
  {
    NREM3 = 0;
    NREM2 = 1;
    NREM1 = 0;
    REM = 0;
    awake = 0;
  }
  else if (avgAccelMovement > 2 && avgAccelMovement <= 3)
  {
    NREM3 = 0;
    NREM2 = 0;
    NREM1 = 1;
    REM = 0;
    awake = 0;
  }
  else if (avgAccelMovement > 3 && avgAccelMovement <= 4)
  {
    NREM3 = 0;
    NREM2 = 0;
    NREM1 = 0;
    REM = 1;
    awake = 0;
  }
  else if (avgAccelMovement > 4 && avgAccelMovement <= 5)
  {
    NREM3 = 0;
    NREM2 = 0;
    NREM1 = 0;
    REM = 0;
    awake = 1;
  }
} // end detSleepStage

void runAccel()
{
  //should start accelerometer sensor readings
  //read sleepDuration and sleepCycles here
  if (sleepDuration<=745 || sleepCycles==1 || sleepCycles==2)
  {
    sleepQuality = 3;
  }
  else if (sleepDuration>=745 && (sleepCycles == 3 || sleepCycles == 4))
  {
    sleepQuality = 2;
  }
  else if (sleepDuration>=755 && sleepCycles >= 5)
  {
    sleepQuality = 1;
  }
} //end runAccel

void selectSong()
{
  switch(sleepQuality)        // sleep quality 1 = above average, 2 = average, 3 = below average
  {
    case 1:
    AAvgSong++;            // plays next song, if AAvgSong= 0+1=1 then plays song 1
    playAAvgSong();
    break;
    case 2:
    avgSong++;
    playAvgSong();
    break;
    case 3:
    BAvgSong++;
    playBAvgSong();
    break;
  }
}

void playAAvgSong()
{
  switch(AAvgSong)
  {
    case 1:
    break;
    case 2:
    break;
    case 3:
    break;
    case 4:
    break;
    case 5:
    break;
    case 6:
    break;
    case 7:
    break;
    case 8:
    break;
    case 9:
    break;
    case 10:
    AAvgSong = 0;
    break;
  }
}

void playAvgSong()
{
  switch(avgSong)
  {
    case 1:
    break;
    case 2:
    break;
    case 3:
    break;
    case 4:
    break;
    case 5:
    break;
    case 6:
    break;
    case 7:
    break;
    case 8:
    break;
    case 9:
    break;
    case 10:
    avgSong = 0;
    break;
  }
}

void playBAvgSong()
{
  switch(BAvgSong)
  {
    case 1:
    break;
    case 2:
    break;
    case 3:
    break;
    case 4:
    break;
    case 5:
    break;
    case 6:
    break;
    case 7:
    break;
    case 8:
    break;
    case 9:
    break;
    case 10:
    BAvgSong = 0;
    break;
  }
} // end playBAvgSong
