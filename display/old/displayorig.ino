/*
Dream Machine Alarm Clock
Display code
v2.0
*/

// ======================== LIBS ==============================

// #include <Wire.h>
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

/*#include "application.h"
#include "DS1307/DS1307.h"*/

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


// Keep track of the hours, minutes, seconds displayed by the clock.
// Start off at 0:00:00 as a signal that the time should be read from
// the DS1307 to initialize it.
int hours = 0;
int minutes = 0;
int seconds = 0;
int earlyAlarmHours=0;       //saves slider 1 value in early alarm hours
int earlyAlarmMinutes=0;     //saves slider 2 value in early alarm minutes
int latestAlarmHours=0;
int latestAlarmMinutes=0;

char sleepquality=0;
// Remember if the colon was drawn on the display so it can be blinked
// on and off every second.
bool blinkColon = false;

int slider1 = A0;
// #define slider1 A0      //Inputs of each slider
#define slider2 A1
#define slider3 A2
#define slider4 A3
#define slideralarm A4
#define inSnooze A5   //Input Snooze

int valslider1=0, pastvalslider1=0;      //value slider and past value slider variables
int valslider2=0, pastvalslider2=0;
int valslider3=0, pastvalslider3=0;
int valslider4=0, pastvalslider4=0;
int valslideralarm=0;                   //value alarm slider

bool fEarlyAlarmOn=0;                //earliest alarm on flag
bool fLatestAlarmOn=0;               //latest alarm on flag
bool fSliderAlarmOn=0;               //Slider alarm ON flag
bool snooze=0;                       //snooze flag

int displayValue=0;           //hour
int displayValuenow=0;        //actual hour in XXXX format i.e. 1234 = 12:34
int displayValuealarmearly=0; //alarm earliest hour in XXXX format i.e. 1234 = 12:34
int displayValuealarmlate=0;  //alarm latest hour in XXXX format i.e. 1234 = 12:34
int displayValuesnooze=0;     //snooze hour in XXXX format i.e. 1234 =12:34

int lsong=0,hsong=0,msong=0;
int sleep_duration=0, sleep_cycles=0;

// ======================== SETUP ==============================

void setup()
{
  Serial.begin(9600);

  matrix.begin(0x70);
  matrix.setBrightness(15);
  Time.zone(-4);
/*
  #ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
  #endif*/

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

  // Setup the DS1307 real-time clock.
  rtc.begin();
  // Set the DS1307 clock if it hasn't been set before.
  //bool setClockTime = !rtc.isrunning();
  // Alternatively you can force the clock to be set again by
  // uncommenting this line:
  //setClockTime = true;
  /*if (setClockTime) {
    Serial.println("Setting DS1307 time!");
    // This line sets the DS1307 time to the exact date and time the
    // sketch was compiled:
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Alternatively you can set the RTC with an explicit date & time,
    // for example to set January 21, 2014 at 3am you would uncomment:
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }*/
} // end setup

// ======================== LOOP ==============================

void loop()
{
// slider1 control



  //displayTime();
  //Serial.println("made it to loop");
  //checkalarms();
  readsliders();                  //calls read sliders function
  if(pastvalslider1!=valslider1)  //if past value of slider 1 is different from the actual value of slider which occurs when user moves the slider 1 (hour of earliest alarm)
  {
    showearliestalarm();          //display shows hour and minutes of earliest alarm
  }
  else if(pastvalslider2!=valslider2)
  {
    showearliestalarm();
  }
  else if(pastvalslider3!=valslider3)
  {
    showlatestalarm();
  }
  else if(pastvalslider4!=valslider4)
  {
    showlatestalarm();
  }
  if(fEarlyAlarmOn==1&&fSliderAlarmOn==1)  //if earliest alarm on flag is 1 and slider alarm on flag is 1 then turn ON earliest alarm
  {
    turnonearliestalarm();
  }
  if(fLatestAlarmOn==1&&fSliderAlarmOn==1)
  {
    turnonlatestalarm();
  }
  if(snooze==1&&displayValuesnooze==displayValuenow)
  {
    turnonsnoozealarm();
  }

  //updatesliders();                        //updates values of sliders
} //end loop

// ======================== ADD'L FUNCTIONS ============================

void displayTime()
{
  DateTime now = rtc.now(); //reads the time now from RTC
  matrix.drawColon(true);
  int hour = now.hour();
  int minute = now.minute();
  int digit0 = hour/10;
  int digit1 = hour % 10;
  int digit3 = minute/10;
  int digit4 = minute % 10;
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.println();
  if (digit0 != 0) {
  matrix.writeDigitNum(0,digit0);
  }
  matrix.writeDigitNum(1,digit1);
  matrix.writeDigitNum(3,digit3);
  matrix.writeDigitNum(4,digit4);
  matrix.writeDisplay();
  delay(1000);

  // Show the time on the display by turning it into a numeric
  // value, like 3:30 turns into 330, by multiplying the hour by
  // 100 and then adding the minutes.

  displayValue = hours*100 + minutes;

  // Do 24 hour to 12 hour format conversion when required.
  if (!TIME_24_HOUR) {
    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
    if (hours > 12) {
      displayValue -= 1200;   //displayValue = displayValue-1200
    }
    // Handle hour 0 (midnight) being shown as 12.
    else if (hours == 0) {
      displayValue += 1200;
    }
  }
  displayValuenow=displayValue; //saves display value into display value now variable

    // Now print the time value to the display.
  clockDisplay.print(displayValuenow, DEC);
  Serial.println(displayValuenow);
  // Add zero padding when in 24 hour mode and it's midnight.
  // In this case the print function above won't have leading 0's
  // which can look confusing.  Go in and explicitly add these zeros.
  if (TIME_24_HOUR && hours == 0)
      {
      // Pad hour 0.
      clockDisplay.writeDigitNum(1, 0);
      // Also pad when the 10's minute is 0 and should be padded.
      if (minutes < 10)
          {
            clockDisplay.writeDigitNum(2, 0);
          }
      }

} //end displayTime


void readsliders()
{
  int digit0;
  int digit1;
  //slider 1 is hour in earliest alarm
  valslider1=analogRead(slider1);            //read slider1 A0...A5 and saves in value slider 1 variable
  valslider1=map( valslider1, 0, 4100, 1 , 13 );  //value slider1 0 to 1023 equivalent 1 to 12
 if (valslider1 < 10){
   digit0 = 0;
   digit1 = valslider1 % 10;
 }
  else
  {
    digit0 = valslider1/10;
    digit1 = valslider1 % 10;
  }

  if (digit0 != 0) {
  matrix.writeDigitNum(0,digit0);
  }
  else
  {
    matrix.writeDigitNum(0,0);
  }
  matrix.writeDigitNum(1,digit1);
//  matrix.writeDigitNum(3,0);
//  matrix.writeDigitNum(4,0);
  matrix.writeDisplay();
  delay(100);

  //slider 2 is minutes in earliest alarm
  valslider2=analogRead(slider2);           //read slider2 A0...A5 and saves in value slider 1 variable
  valslider2=map(valslider2,0,4100,0,59);   //value slider2 0 to 1023 equivalent 0 to 59

  //slider 3 is hour in latest alarm
  valslider3=analogRead(slider3);           //read slider3 A0...A5 and saves in value slider 1 variable
  valslider3=map(valslider3,0,4100,1,12);   //value slider3 0 to 1023 equivalent 1 to 12

  //slider 4 is minutes in latest alarm
  valslider4=analogRead(slider4);           //read slider4 A0...A5 and saves in value slider 1 variable
  valslider4=map(valslider4,0,4100,0,59);   //value slider4 0 to 1023 equivalent 0 to 59

  earlyAlarmHours=valslider1;       //saves slider 1 value in early alarm hours
  earlyAlarmMinutes=valslider2;     //saves slider 2 value in early alarm minutes
  latestAlarmHours=valslider3;
  latestAlarmMinutes=valslider4;

  displayValuealarmearly=earlyAlarmHours*100+earlyAlarmMinutes;
  displayValuealarmlate=latestAlarmHours*100+latestAlarmMinutes;

}

void checkalarms()
{
  if(displayValuenow==displayValuealarmearly)fEarlyAlarmOn=1;//if the hour and minute are equal to alarm hour and minute
                                                       //then turn on alarm earliest flag
  else if(displayValuenow!=displayValuealarmearly)fEarlyAlarmOn=0;//if not then turn off alarm earliest flag

  if(displayValuenow==displayValuealarmlate)fLatestAlarmOn=1;

  else if(displayValuenow!=displayValuealarmlate)fLatestAlarmOn=0;

}

void showearliestalarm()
{
  char countersec=0; //use char so it can save until value 255
  do
  {
    displayearlyalarm();
    readsliders();    //read value sliders to check if user is not moving slider
    if(pastvalslider1!=valslider1||pastvalslider2!=valslider2)countersec=0;   //if past sliders 1 or 2 values have changed
                                                                              //user is still moving sliders then reset counter
    delay(10);        //wait 10 milis
    countersec++;     //adds 1 to counter seconds
    updatesliders();  //updates sliders values
  }
  while(countersec<200||pastvalslider1!=valslider1||pastvalslider2!=valslider2);
                        //while user moves slider 1 or slider 2 or counter is less than 2 sec
                        //10 milis x 200= 2 seconds it shows in display and then return to actual time
                       //change 200 value if less or more time is desired
}

void showlatestalarm()
{
  char countersec=0; //use char so it can save until value 255
  do
  {
    displaylatealarm();
    readsliders();    //read value sliders to check if user is not moving slider
    if(pastvalslider3!=valslider3||pastvalslider4!=valslider4)countersec=0;   //if past sliders 1 or 2 values have changed
                                                                              //user is still moving sliders then reset counter
    delay(10);        //wait 10 milis
    countersec++;     //adds 1 to counter seconds
    updatesliders();  //updates sliders values
  }
  while(countersec<200||pastvalslider3!=valslider3||pastvalslider4!=valslider4);
                        //while user moves slider 1 or slider 2 or counter is less than 2 sec
                        //10 milis x 200= 2 seconds it shows in display and then return to actual time
                       //change 200 value if less or more time is desired
}

void turnonearliestalarm()
{
  readaccel();          //this subrutine reads the accel and evaluates sleep quality
  sleepanalyzer();       //chooses the right song depending on sleep quality
  snooze=0;                           //resets snooze flag
  while(snooze!=1&&fEarlyAlarmOn==1)
  {

    snooze=analogRead(inSnooze);
    snooze=map(snooze,0,1023,0,1);
    if (snooze==1)
    {
      savesnoozehour();
    }
    valslideralarm=analogRead(slideralarm);
    valslideralarm=map(valslideralarm,0,1023,0,1);
    if(valslideralarm==0)fEarlyAlarmOn==0;
    //Insert music and accel values or decisions here
  }
}

void turnonlatestalarm()
{

  readaccel();          //this subrutine reads the accel and evaluates sleep quality
  sleepanalyzer();       //plays song choosen from algorithm
  snooze=0;                           //resets snooze flag
  while(snooze!=1&&fLatestAlarmOn==1)
  {

    snooze=analogRead(inSnooze);
    snooze=map(snooze,0,1023,0,1);
    if (snooze==1)
    {
      savesnoozehour();
    }
    valslideralarm=analogRead(slideralarm);
    valslideralarm=map(valslideralarm,0,1023,0,1);
    if(valslideralarm==0)fLatestAlarmOn==0;

  }
}

void updatesliders()
{
    pastvalslider1=valslider1;    //update sliders value
    pastvalslider2=valslider2;
    pastvalslider3=valslider3;
    pastvalslider4=valslider4;
}

void displayearlyalarm()
{
  // Do 24 hour to 12 hour format conversion when required.
  if (!TIME_24_HOUR)
  {
    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
    if (earlyAlarmHours > 12)
    {
      displayValuealarmearly -= 1200;   //displayValue = displayValue-1200
    }
    // Handle hour 0 (midnight) being shown as 12.
    else if (earlyAlarmHours == 0)
    {
      displayValuealarmearly += 1200;
    }
  }

    // Now print the time value to the display.
  clockDisplay.print(displayValuealarmearly, DEC);

  // Add zero padding when in 24 hour mode and it's midnight.
  // In this case the print function above won't have leading 0's
  // which can look confusing.  Go in and explicitly add these zeros.
  if (TIME_24_HOUR && earlyAlarmHours == 0)
      {
      // Pad hour 0.
      clockDisplay.writeDigitNum(1, 0);
      // Also pad when the 10's minute is 0 and should be padded.
      if (earlyAlarmMinutes < 10)
          {
            clockDisplay.writeDigitNum(2, 0);
          }
      }
}

void displaylatealarm()
{
  // Do 24 hour to 12 hour format conversion when required.
  if (!TIME_24_HOUR)
  {
    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
    if (latestAlarmHours > 12)
    {
      displayValuealarmlate -= 1200;   //displayValue = displayValue-1200
    }
    // Handle hour 0 (midnight) being shown as 12.
    else if (latestAlarmHours == 0)
    {
      displayValuealarmlate += 1200;
    }
  }

    // Now print the time value to the display.
  clockDisplay.print(displayValuealarmlate, DEC);

  // Add zero padding when in 24 hour mode and it's midnight.
  // In this case the print function above won't have leading 0's
  // which can look confusing.  Go in and explicitly add these zeros.
  if (TIME_24_HOUR && latestAlarmHours == 0)
      {
      // Pad hour 0.
      clockDisplay.writeDigitNum(1, 0);
      // Also pad when the 10's minute is 0 and should be padded.
      if (latestAlarmMinutes < 10)
          {
            clockDisplay.writeDigitNum(2, 0);
          }
      }
}

void turnonsnoozealarm()
{
  readaccel();          //this subrutine reads the accel and evaluates sleep quality
  sleepanalyzer();       //plays song choosen from algorithm
  snooze=0;                           //resets snooze flag
  while(snooze!=1&&valslideralarm==1) //check if snooze was pushed again or slider alarm on is off to break
  {

    snooze=analogRead(inSnooze);
    snooze=map(snooze,0,1023,0,1);
    if (snooze==1)                //if snooze was pushed again save and add 10 minutes to actual time
    {
      savesnoozehour();
    }
    valslideralarm=analogRead(slideralarm);
    valslideralarm=map(valslideralarm,0,1023,0,1);
  }
}

void savesnoozehour()
{
  displayValuesnooze=displayValuenow;     //save actual time
  displayValuesnooze+=10;                 //add 10 minutes if needed more or less minutes change the number
}

void sleepanalyzer()
{
  switch(sleepquality)        //sleep quality 1 = above average, 2= average, 3=below average
  {
    case 1:
    lsong++;            //plays next song, if lsong= 0+1=1 then plays song 1
    playlightsong();
    break;
    case 2:
    hsong++;
    playheavysong();
    break;
    case 3:
    msong++;
    playmetalsong();
    break;
  }
}

void readaccel()
{
  //read sleep_duration and sleep_cycles here

  if(sleep_duration<=745||sleep_cycles==1||sleep_cycles==2)sleepquality=3;
  else if(sleep_cycles==3)sleepquality=2;
  else if(sleep_cycles>=4)sleepquality=1;
}

void playlightsong()
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

void playheavysong()
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

void playmetalsong()
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
}
