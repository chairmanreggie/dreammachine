/*
Dream Machine Alarm Clock
Display Code - runs a 7 segment display using multiple sliders as alarm inputs
Author: Reggie Raye
Release date: 5/16/17
v2.1
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
int earlyAlarmHours = 0;       //saves slider 1 value in early alarm hours
int earlyAlarmMinutes = 0;     //saves slider 2 value in early alarm minutes
int latestAlarmHours = 0;
int latestAlarmMinutes = 0;

char sleepQuality=0;

bool blinkColon = false;

int slider1 = A0;
int slider2 = A1;
int slider3 = A2;
int slider4 = A3;

#define slider1 A0      //Inputs of each slider
#define slider2 A1
#define slider3 A2
#define slider4 A3
#define slideralarm A4
#define inSnooze A5   //Input Snooze
#define ampmEarly D3        //am or pm selection alarms
#define ampmLate D4

int valslider1=0, pastvalslider1=0;      //value slider and past value slider variables
int valslider2=0, pastvalslider2=0;
int valslider3=0, pastvalslider3=0;
int valslider4=0, pastvalslider4=0;
int valslideralarm = 0;                   //value alarm slider

int digit0;
int digit1;
int digit3;
int digit4;

bool fEarlyAlarmOn=0;                //earliest alarm on flag
bool fLatestAlarmOn=0;               //latest alarm on flag
bool fSliderAlarmOn=0;               //Slider alarm ON flag
bool snooze = 0;                       //snooze flag
bool AMorPMearly=0;                         //am or pm flag if ==1 then am if ==0 then pm
bool AMorPMlate=0;                         //am or pm flag if ==1 then am if ==0 then pm
bool AM=0;                                 //actual time flag
bool PM=0;

int displayValue=0;           //hour
int displayValueNow=0;        //actual hour in XXXX format i.e. 1234 = 12:34
int displayValueAlarmEarly=0; //alarm earliest hour in XXXX format i.e. 1234 = 12:34
int displayValueAlarmLate=0;  //alarm latest hour in XXXX format i.e. 1234 = 12:34
int displayValuesnooze=0;     //snooze hour in XXXX format i.e. 1234 =12:34

int lsong=0,hsong=0,msong=0;
int sleep_duration=0, sleep_cycles=0;

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
  //checkAlarms(); //not sure if needed
  readSliders();

  if (pastvalslider1 != valslider1 || pastvalslider2 != valslider2)  //if past value of slider 1 or value slider 2 is different from the actual value of slider which occurs when user moves the sliders (hour of earliest alarm)
  {
    showEarliestAlarm();          //display shows hour and minutes of earliest alarm
  }
  else if (pastvalslider3 != valslider3||pastvalslider4 != valslider4)
  {
    showLatestAlarm();
  }
  if (fEarlyAlarmOn==1 && fSliderAlarmOn==1&&((AM==1&&AMorPMearly==1)||(PM==1&&AMorPMearly==0)))  //if earliest alarm on flag is 1 and slider alarm on flag is 1 then turn ON earliest alarm
                                              //here we compare if AM=1 and AMorPMearly=1(this means alarm is on AM Option) then we turn on alarm
  {                                           //OR if PM=1 and AMorPMearly=0(this means alarm is on PM Option) then we turn on alarm
    turnOnEarliestAlarm();
  }
  if (fLatestAlarmOn==1 && fSliderAlarmOn==1&&((AM==1&&AMorPMlate==1)||(PM==1&&AMorPMlate==0))) //same as past one just with Late flags
  {
    turnOnLatestAlarm();
  }
  if (snooze==1 && displayValuesnooze==displayValueNow)
  {
    turnOnSnoozeAlarm();
  }

  //make pastvalslider == valslider
  updateSliders();


} //end loop

// ======================== ADD'L FUNCTIONS ============================

void displayTime()
{
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();

  if (hour < 10)
  {
  digit1 = hour;
  }
  else if (hour == 10 || hour == 11 || hour == 12)
  {
  digit0 = 1;
  digit1 = hour % 10;
  }
  else if (hour > 12)
  {
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

  Serial.println("displaytime running:");
  Serial.print(digit0);
  Serial.print(digit1);
  Serial.print(":");
  Serial.print(digit3);
  Serial.print(digit4);
  Serial.println();

  matrix.drawColon(true);
  if (digit0 == 1)
  {
  matrix.writeDigitNum(0,digit0);
  }
  else if (digit0 != 1)
  {
  matrix.clear();
  }
  matrix.writeDigitNum(1,digit1);
  matrix.writeDigitNum(3,digit3);
  matrix.writeDigitNum(4,digit4);
  matrix.writeDisplay();

  delay(1000);

} //end displayTime

void readSliders()
{
//========= SLIDER1 (earliest hours) ========

  valslider1 = analogRead(slider1);
  valslider1 = map(valslider1, 0, 4096, 1 , 12);

//========= SLIDER2 (earliest minutes) ========

  valslider2 = analogRead(slider2);
  valslider2 = 5*map(valslider2, 0, 4096, 0, 12);

//========= SLIDER3 (latest hour) ========

  valslider3 = analogRead(slider3);
  valslider3 = map(valslider3, 0, 4096, 1, 12);

//========= SLIDER4 (latest minute) ========

  valslider4 = analogRead(slider4);
  valslider4 = 5*map(valslider4, 0, 4096, 0, 12);

  //check if early alarm and late alarm are on PM or AM option
  if (digitalRead(ampmEarly) == HIGH)
  {
    AMorPMearly = 1;
  }
  else if (digitalRead(ampmEarly) == LOW)
  {
    AMorPMearly = 0;
  }
  if (digitalRead(ampmLate) == HIGH)
  {
    AMorPMlate = 1;
  }
  else if (digitalRead(ampmLate) == LOW)
  {
    AMorPMlate = 0;
  }

  earlyAlarmHours = valslider1;
  earlyAlarmMinutes = valslider2;
  latestAlarmHours = valslider3;
  latestAlarmMinutes = valslider4;

  displayValueAlarmEarly = earlyAlarmHours*100 + earlyAlarmMinutes;
  displayValueAlarmLate = latestAlarmHours*100 + latestAlarmMinutes;

} // end readSliders

void checkAlarms()
{
  if (displayValueNow == displayValueAlarmEarly)
  {
     fEarlyAlarmOn = 1; //if the hour and minute are equal to alarm hour and minute then turn on alarm earliest flag
  }
  else if(displayValueNow != displayValueAlarmEarly)
  {
    fEarlyAlarmOn = 0; //if not then turn off alarm earliest flag
  }
  if (displayValueNow == displayValueAlarmLate)
  {
     fLatestAlarmOn = 1;
  }
  else if(displayValueNow != displayValueAlarmLate)
  {
    fLatestAlarmOn = 0;
  }
} // end checkAlarms

void showEarliestAlarm()
{
  char countersec = 0; //use char so it can save until value 255
  do
  {
    displayEarlyAlarm();
    readSliders();    //read value sliders to check if user is not moving slider
    if (pastvalslider1 != valslider1 || pastvalslider2 != valslider2)  //if past sliders 1 or 2 values have changed
    {
      countersec = 0;
    } //user is still moving sliders then reset counter
    delay(10);        //wait 10 milis
    countersec++;     //adds 1 to counter seconds
    updateSliders();  //updates sliders values
  }
  while (countersec < 200);

} // end showEarliestAlarm

void showLatestAlarm()
{
  char countersec=0; //use char so it can save until value 255
  do
  {
    displayLateAlarm();
    readSliders();    //read value sliders to check if user is not moving slider
    if(pastvalslider3!=valslider3||pastvalslider4!=valslider4)countersec=0;   //if past sliders 1 or 2 values have changed
                                                                              //user is still moving sliders then reset counter
    delay(10);        //wait 10 milis
    countersec++;     //adds 1 to counter seconds
    updateSliders();  //updates sliders values
  }
  while(countersec<100 /*|| pastvalslider3!=valslider3 || pastvalslider4!=valslider4*/); //just erased here because counter sec will update if sliders move
                        //while user moves slider 1 or slider 2 or counter is less than 2 sec
                        //10 milis x 200= 2 seconds it shows in display and then return to actual time
                       //change 200 value if less or more time is desired
} // end showLatestAlarm

void turnOnEarliestAlarm()
{
  readAccel();          //this subroutine reads the accel and evaluates sleep quality
  sleepAnalyzer();       //chooses the right song depending on sleep quality
  snooze = 0;                           //resets snooze flag
  while(snooze!=1 && fEarlyAlarmOn==1)
  {
    snooze = analogRead(inSnooze);
    snooze = map(snooze,0,4096,0,1);
    if (snooze == 1)
    {
      saveSnoozeHour();
    }
    valslideralarm = analogRead(slideralarm);
    valslideralarm = map(valslideralarm,0,4096,0,1);
    if (valslideralarm == 0)
    {
     fEarlyAlarmOn = 0;
    }

    int eeprom_address = 10;
    struct saveEarliestWakeupTime {
      uint8_t valslider1;
      uint8_t valslider2;
    };
    saveEarliestWakeupTime earliestWakeup;

    //Insert music and accel values or decisions here
  }
} // end turnOnEarliestAlarm

void turnOnLatestAlarm()
{
  readAccel();          //this subroutine reads the accel and evaluates sleep quality
  sleepAnalyzer();       //plays song choosen from algorithm
  snooze = 0;                           //resets snooze flag
  while (snooze!=1 && fLatestAlarmOn==1)
  {
    snooze = analogRead(inSnooze);
    snooze = map(snooze,0,4096,0,1);
    if (snooze==1)
    {
      saveSnoozeHour();
    }
    valslideralarm = analogRead(slideralarm);
    valslideralarm = map(valslideralarm,0,4096,0,1);
    if (valslideralarm==0)
    {
      fLatestAlarmOn = 0;
    }
  } //end while
} //end turnOnLatestAlarm

void updateSliders()
{
    pastvalslider1 = valslider1;    //update slider value
    pastvalslider2 = valslider2;
    pastvalslider3 = valslider3;
    pastvalslider4 = valslider4;
}

void displayEarlyAlarm()
{
  if (valslider1 < 10)
  {
   digit0 = 0;
   digit1 = valslider1 % 10;
  }
  else if(valslider1 >= 10)
  {
   digit0 = valslider1/10;
   digit1 = valslider1 % 10;
  }
  if (valslider2 < 10)
  {
   digit3 = 0;
   digit4 = valslider2;
  }
  else if (valslider2 >= 10)
  {
   digit3 = valslider2/10;
   digit4 = valslider2 % 10;
  }

  //matrix.drawColon(true);
  if (digit0 == 1)
  {
  matrix.writeDigitNum(0,digit0);
  }
  else if (digit0 != 1)
  {
  matrix.clear();
  }
  matrix.writeDigitNum(1,digit1);
  matrix.writeDigitNum(3,digit3);
  matrix.writeDigitNum(4,digit4);
  matrix.writeDisplay();

} //end displayEarlyAlarm

void displayLateAlarm()
{
  if (valslider3 < 10)
  {
   digit0 = 0;
   digit1 = valslider3 % 10;
  }
  else if (valslider3 >= 10)
  {
   digit0 = valslider3/10;
   digit1 = valslider3 % 10;
  }

  if (valslider4 < 10)
  {
   digit3 = 0;
   digit4 = valslider4;
  }
  else if (valslider4 >= 10)
  {
   digit3 = valslider4/10;
   digit4 = valslider4 % 10;
  }

  //matrix.drawColon(true);
  if (digit0 == 1)
  {
  matrix.writeDigitNum(0,digit0);
  }
  else if (digit0 != 1)
  {
  matrix.clear();
  }
  matrix.writeDigitNum(1,digit1);
  matrix.writeDigitNum(3,digit3);
  matrix.writeDigitNum(4,digit4);
  matrix.writeDisplay();

} //end displayLateAlarm

void turnOnSnoozeAlarm()
{
  readAccel();          //this subroutine reads the accel and evaluates sleep quality
  sleepAnalyzer();       //plays song choosen from algorithm
  snooze=0;                           //resets snooze flag
  while(snooze!=1 && valslideralarm==1) //check if snooze was pushed again or slider alarm on is off to break
  {
    snooze = analogRead(inSnooze);
    snooze = map(snooze,0,4096,0,1);
    if (snooze==1)                //if snooze was pushed again save and add 10 minutes to actual time
    {
      saveSnoozeHour();
    }
    valslideralarm = analogRead(slideralarm);
    valslideralarm = map(valslideralarm,0,4096,0,1);
  }
}

void saveSnoozeHour()
{
  displayValuesnooze = displayValueNow;     //save actual time
  displayValuesnooze+=10;                 //add 10 minutes if needed more or less minutes change the number
}

void sleepAnalyzer()
{
  switch(sleepQuality)        //sleep quality 1 = above average, 2= average, 3=below average
  {
    case 1:
    lsong++;            //plays next song, if lsong= 0+1=1 then plays song 1
    playLightSong();
    break;
    case 2:
    hsong++;
    playMediumSong();
    break;
    case 3:
    msong++;
    playHeavySong();
    break;
  }
}

void readAccel()
{
  //read sleep_duration and sleep_cycles here
  if(sleep_duration<=745 || sleep_cycles==1 || sleep_cycles==2)
  {
    sleepQuality = 3;
  }
  else if (sleep_cycles == 3)
  {
    sleepQuality = 2;
  }
  else if (sleep_cycles >= 4)
  {
    sleepQuality = 1;
  }
} //end readAccel

void playLightSong()
{
  switch(lsong) //light song
  {
    case 1:
    //play song 1
    break;

    case 2:
    //play song 2
    break;

    case 3:
    //play song 3
    break;

    case 4:
    //play song 4
    break;

    case 5:
    //play song 5
    break;

    case 6:
    //play song 6
    break;

    case 7:
    //play song 7
    break;

    case 8:
    //play song 8
    break;

    case 9:
    //play song 9
    break;

    case 10:
    //play song 10 and resets lsong
    lsong=0;
    break;
  }
}

void playMediumSong()
{
  switch(hsong) //heavy song depending on counter hsong
  {
    case 1:
    //play song 1
    break;

    case 2:
    //play song 2
    break;

    case 3:
    //play song 3
    break;

    case 4:
    //play song 4
    break;

    case 5:
    //play song 5
    break;

    case 6:
    //play song 6
    break;

    case 7:
    //play song 7
    break;

    case 8:
    //play song 8
    break;

    case 9:
    //play song 9
    break;

    case 10:
    //play song 10 and resets lsong
    hsong=0;
    break;

  }
}

void playHeavySong()
{
  switch(msong) //metal song
  {
    case 1:
    //play song 1
    break;

    case 2:
    //play song 2
    break;

    case 3:
    //play song 3
    break;

    case 4:
    //play song 4
    break;

    case 5:
    //play song 5
    break;

    case 6:
    //play song 6
    break;

    case 7:
    //play song 7
    break;

    case 8:
    //play song 8
    break;

    case 9:
    //play song 9
    break;

    case 10:
    //play song 10 and resets lsong
    msong=0;
    break;

  }
} // end playHeavySong
