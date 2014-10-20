// This code will output GPS data from the Ultimate GPS Breakout V3 (Adafruit ID: 746).
// By: Spencer McDonald
// 2/19/14

// To connect the GPS Breakout: VIN = 3.3, GND = GND, RX = 5, TX = 6.
// To connect the SD/RTC shield, plug it in.

// The pins used in this code are A4/A5 (I2C),D5,6,10-13.

// Libraries to include

#include "SD.h"                    // SD library
#include <Wire.h>                  // RTC library
#include "RTClib.h"

#include "TinyGPS.h"               // GPS Library
#include <SoftwareSerial.h>

//Define Digital LED Pins
#define redLEDpin 3
#define greenLEDpin 4

// Initalize Variables

#define LOGTIME 1000 // In miliseconds

float Latitude;
float Longitude;
float alt;
int sat;
float gpsheading;
float velocity;
unsigned long FixAge;

const int chipSelect = 10; // Sparkfun microSD uses 8, Adafruit uses 10

File datalog;
char filename[] = "LOGGER00.csv";
RTC_DS1307 RTC;

// Initalize GPS

  TinyGPS gps;
  SoftwareSerial ss(5, 6);


static bool feedgps(void);

void setup () {
    Serial.begin(115200);
    ss.begin(9600);  // Opens the communication between Arduino and GPS

    Wire.begin();         // Begins RTC communication
    RTC.begin();

// Check if RTC is running, if not fix it

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

// Initalize SD Card

  Serial.print("Intializing SD Card...");
  pinMode(10, OUTPUT);

  if(!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("Card initialized.");

  Serial.print("Creating File...");


// Make a new file each time the arduino is powered

  for (uint8_t i = 0; i < 100; i++)
  {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';

    if (! SD.exists(filename))
    {
      // only open a new file if it doesn't exist
      datalog = SD.open(filename, FILE_WRITE);
      break;
    }
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  if(!datalog)
  {
  Serial.println("Couldn't Create File");

  return;
  }

// Print Header

  String Header = "RTC Date, RTC Time, Lat, Lon, Alt (ft), # of Sat";

  datalog = SD.open(filename, FILE_WRITE);
  datalog.println(Header);
  Serial.println(Header);
  datalog.close();

  pinMode(greenLEDpin, OUTPUT);
}

void loop () {

 delay(500);
 digitalWrite(greenLEDpin, LOW);

 bool newdata = false;
 unsigned long start = millis();

 while((millis() - start) < (LOGTIME))
 {
    if (feedgps())
    newdata = true;
 }

 datalog = SD.open(filename, FILE_WRITE);

 // Real Time Clock

  DateTime now = RTC.now();

  datalog.print(now.month(), DEC);
  datalog.print("/");
  datalog.print(now.day(), DEC);
  datalog.print("/");
  datalog.print(now.year(), DEC);
  datalog.print(",");
  datalog.print(now.hour(), DEC);
  datalog.print(":");
  datalog.print(now.minute(), DEC);
  datalog.print(":");
  datalog.print(now.second(), DEC);
  datalog.print(",");

  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print("/");
  Serial.print(now.year(), DEC);
  Serial.print(", ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print(", ");

// GPS

  gps.f_get_position(&Latitude, &Longitude, &FixAge);

  sat=gps.satellites();
  alt=gps.f_altitude();
  gpsheading=gps.f_course();
  velocity=gps.f_speed_kmph();

  datalog.print(Latitude,5);
  datalog.print(",");
  datalog.print(Longitude,5);
  datalog.print(",");
  datalog.print(alt*3.28084); // m to ft
  datalog.print(",");
  datalog.print(sat);
  datalog.print(",");
  //datalog.print(gpsheading);
  //datalog.print(",");
  //datalog.print(velocity,3); // in m/s
  //datalog.print(",");

  Serial.print(Latitude,5);
  Serial.print(", ");
  Serial.print(Longitude,5);
  Serial.print(", ");
  Serial.print(alt*3.28084); // m to ft
  Serial.print(", ");
  Serial.print(sat);
  Serial.print(", ");
  //Serial.print(gpsheading);
  //Serial.print(", ");
  //Serial.print(velocity*3.28084,3); // in m/s
  //Serial.print(", ");

  // New line for new datapoints

  datalog.println();
  Serial.println();
  datalog.close();

  digitalWrite(greenLEDpin, HIGH);  //Flashes LED to let us know that it is running
  delay(500);


}

static bool feedgps()
{
  while (ss.available())
  {
    if (gps.encode(ss.read()))
      return true;
  }
  return false;
}
