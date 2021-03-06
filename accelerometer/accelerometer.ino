/*
Dream Machine Alarm Clock
v2.0
*/

// ======================== LIBS ==============================

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

// ======================== VARS ==============================

Adafruit_MMA8451 mma = Adafruit_MMA8451();

// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
const int numReadings = 10;
const int numaverages=10;   //number of times read raw cycles and averages changing the value makes it more sensitive

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

const int moduledots=10;        //dots per module adjust as desired
int graphdots=0;                //dots in graphic

int xdots[moduledots];           //xdots array
int ydots[moduledots];           //ydots array
int zdots[moduledots];           //zdots array

int inputPin = A0;              //measure analog pin
int xpeak=0,ypeak=0,zpeak=0;                     //peak and trough index variable
int xtrough=0,ytrough=0,ztrough=0;

int xtotal = 0;
int xaverage = 0;
int delta_x = 0;
int xreadings[numReadings];

int ytotal = 0;
int yaverage = 0;
int yreadings[numReadings];

int ztotal = 0;
int zaverage = 0;
int zreadings[numReadings];

// ======================== SETUP ==============================

void setup()
{
  // initialize serial communication with computer:
  Serial.begin(9600);
  delay(2000);

  Serial.println("Adafruit MMA8451 test!");
  do {
    Serial.println("Couldnt start");      //send if no sensor is connected then attempt to reconnect
    }
    while (!mma.begin());
  Serial.println("MMA8451 found!");

  mma.setRange(MMA8451_RANGE_2_G);

  Serial.print("Range = "); Serial.print(2 << mma.getRange());
  Serial.println("G");
} // end setup

// ======================== LOOP ==============================

void loop()
{
  smoothRead();                 //smooth readings in module
  xpeak=searchPeak(xdots,10);     //searchs for x peak data in module
  xtrough=searchTrough(xdots,10); //searchs for x trough data in module

  ypeak=searchPeak(ydots,10);     //searchs for y peak data in module
  ytrough=searchTrough(ydots,10); //searchs for y trough data in module

  zpeak=searchPeak(zdots,10);     //searchs for z peak data in module
  ztrough=searchTrough(zdots,10); //searchs for z trough data in module

//  storeIt();                    //stores relevant information - not yet config'd
//  graphIt();                    //graph module data - currently broken
//  sendInfo();                   //sends info via serial - not yet config'd
//  delay(2000);                  //don't think this is necessary..
} // end loop

// ======================== ADD'L FUNCTIONS ============================

void smoothRead()
{
  total=0;
  xaverage=0;
  yaverage=0;       //resets all xyz values so they don't be cumulative
  zaverage=0;

  xtotal=0;
  ytotal=0;
  ztotal=0;
  for(graphdots=0;graphdots<moduledots;graphdots++) //for 10 moduledots smooths readings then it stores data in dots array
  {
    readRaw();
    xdots[graphdots]=xaverage;
    ydots[graphdots]=yaverage;
    zdots[graphdots]=zaverage;
  }
} // end smoothRead

void readRaw()
{
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    readings[thisReading] = 0;
  }
  for (int i=0; i<numaverages;i++)
  {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  mma.read();
  sensors_event_t event;
  mma.getEvent(&event);

    int xRead = event.acceleration.x;
    int yRead = event.acceleration.y;
    int zRead = event.acceleration.z;

        // subtract the last reading:
        xtotal = xtotal - xreadings[readIndex];
        ytotal = ytotal - yreadings[readIndex];
        ztotal = ztotal - zreadings[readIndex];
        // read from the sensor:
        xreadings[readIndex] = event.acceleration.x;
        yreadings[readIndex] = event.acceleration.y;
        zreadings[readIndex] = event.acceleration.z;
        // add the reading to the total:
        xtotal = xtotal + xreadings[readIndex];
        ytotal = ytotal + yreadings[readIndex];
        ztotal = ztotal + zreadings[readIndex];
        // advance to the next position in the array:
        readIndex = readIndex + 1;

        // if we're at the end of the array...
        if (readIndex >= numReadings)
        {
        // ...wrap around to the beginning:
        readIndex = 0;
        }

        // calculate the average:
        xaverage = xtotal / numReadings;
        yaverage = ytotal / numReadings;
        zaverage = ztotal / numReadings;
  } // end for
  // send it to the computer as ASCII digits
  Serial.print("X: ");
  Serial.print(xaverage);
  Serial.print(",");
  Serial.print("Y: ");
  Serial.print(yaverage);
  Serial.print(",");
  Serial.print("Z: ");
  Serial.print(zaverage);
  Serial.println();
  delay(10);                      // delay in between reads for stability
} //end readRaw

int searchPeak(int* array, int size)      // subroutine to find data peak
{
 int maxIndex = 0;
 int max = array[maxIndex];
 for (int i=1; i<size; i++)
 {
   if (max<array[i])
   {
     max = array[i];
     maxIndex = i;
   }
 }
 return maxIndex;
}

int searchTrough(int* array, int size)  // subroutine to find data trough
{
 int minIndex = 0;
 int min = array[minIndex];
 for (int i=1; i<size; i++)
 {
   if (min>array[i])
   {
     min = array[i];
     minIndex = i;
   }
 }
 return minIndex;
}

void storeIt()                      // subroutine to store relevant information
{
}

void graphIt()                    // graph all module if needed and finally peak and trough data
{
  for(graphdots=0;graphdots<moduledots;graphdots++)
  {
    Serial.println(xdots[graphdots]);
  }
  for(graphdots=0;graphdots<moduledots;graphdots++)
  {
    Serial.println(ydots[graphdots]);
  }
  for(graphdots=0;graphdots<moduledots;graphdots++)
  {
    Serial.println(zdots[graphdots]);
  }
    Serial.println(xdots[xpeak]);
    Serial.println(xdots[xtrough]);

    Serial.println(ydots[ypeak]);
    Serial.println(ydots[ytrough]);

    Serial.println(zdots[zpeak]);
    Serial.println(zdots[ztrough]);

} // end graphIt

void sendInfo()                 //send info wherever you want
{
}
