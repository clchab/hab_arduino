// This code will output GPS data from the Ultimate GPS Breakout V3 (Adafruit ID: 746).
// By: Spencer McDonald
// 2/19/14

// To connect the GPS Breakout: VIN = 3.3, GND = GND, RX = 8, TX = 9.
// To connect the SD/RTC shield, plug it in.

// The pins used in this code are A4/A5 (I2C),D5,6,10-13.

// Libraries to include

#include <SD.h>                    // SD library

#include <string.h>

#include <Wire.h>                  // RTC library
#include "RTClib.h"

#include "TinyGPS.h"               // GPS Library
#include <SoftwareSerial.h>

#include "config.h"

// SD card setup
Sd2Card card;
SdVolume volume;
SdFile root;

// audio data logging
File datalog;
File tempfile;
char filename[] = "adlog00.txt";
char bufFile[] = "0000000000.wav";
char sdfile[] = "0000000000.txt";
RTC_DS1307 RTC;

// buf is used to store icoming data and is written to file when full
// 512 bytes is optimized for sdcard
#define BUF_SIZE 512
uint8_t buf[BUF_SIZE];
uint16_t bufcount;
byte wavheader[44];
unsigned long starttime , endtime, filesize;
float frequency = 9.448; //Used for first reading to estimate sample duration
boolean hascard, hasdata, written;
float period , interval;
unsigned long readings = 28672; //  initial sample size- kept small to avoid delay- enough to create data to look at
unsigned long counter;

// GPS
TinyGPS gps;
SoftwareSerial ss(8, 9);

float Latitude;
float Longitude;
float alt;
int sat;
float gpsheading;
float velocity;
unsigned long FixAge;

// led pins
byte yellowLEDpin = 2;
byte redLEDpin = 3;
byte greenLEDpin = 4;

static boolean fileopen();
static bool feedgps(void);
static void error(byte errno);
static void startad();
static void headmod(long value, byte location);

void setup () {
  Serial.begin(115200);
  Serial.flush();
  ss.begin(9600);  // Opens the communication between Arduino and GPS

  // wavheader setup
  // little endian (lowest byte 1st)
  wavheader[0]='R';
  wavheader[1]='I';
  wavheader[2]='F';
  wavheader[3]='F';
  //wavheader[4] to wavheader[7] size of file-2
  //  wavheader[4]='\0';
  //  wavheader[5]='\0';
  //  wavheader[6]='\0';
  //  wavheader[7]='\0';
  // end size of file
  wavheader[8]='W';
  wavheader[9]='A';
  wavheader[10]='V';
  wavheader[11]='E';
  wavheader[12]='f';
  wavheader[13]='m';
  wavheader[14]='t';
  wavheader[15]=' ';
  wavheader[16]=16;
  wavheader[17]=0;
  wavheader[18]=0;
  wavheader[19]=0;
  wavheader[20]=1;
  wavheader[21]=0;
  wavheader[22]=1;
  wavheader[23]=0;
  //wavheader[24] to wavheader[27] samplerate hz
  //  wavheader[24]=0;
  //  wavheader[25]=0;
  //  wavheader[26]=0;
  //  wavheader[27]=0;
  //wavheader[28] to wavheader[31] samplerate*1*1
  //  wavheader[27]=0;
  //  wavheader[28]=0;
  //  wavheader[29]=0;
  //  wavheader[30]=0;
  //  wavheader[31]=0;
  // end wave 28 to 31
  wavheader[32]=1;
  wavheader[33]=0;
  wavheader[34]=8;
  wavheader[35]=0;
  wavheader[36]='d';
  wavheader[37]='a';
  wavheader[38]='t';
  wavheader[39]='a';
  //wavheader[40] to wavheader[43] sample number

  pinMode(greenLEDpin, OUTPUT);
  pinMode(yellowLEDpin, OUTPUT);
  pinMode(redLEDpin, OUTPUT);

  Wire.begin();         // Begins RTC communication
  RTC.begin();

  // Check if RTC is running, if not fix it
  if (! RTC.isrunning()) {
    //Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  // Initalize SD Card
  //Serial.print("Intializing SD Card...");
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    error(1);
    Serial.println("Card failed, or not present");
  } else {
    Serial.println("SD card found");
    digitalWrite(yellowLEDpin, HIGH);
    hascard = true;
  }
  hascard = fileopen();
}

/*
ISR(ADC_vect) {//when new ADC value ready
  if (counter < readings) {
    buf[bufcount] = ADCH;
    counter ++;
    bufcount++;
    if (bufcount == BUF_SIZE) {
      cli();
      bufcount = 0;
      if (!tempfile) error(3);
      tempfile.write(buf, BUF_SIZE);
      sei();
    } else {
      // all data collected
      cli();
      endtime = millis();
      ADCSRA &= ~ADEN;
      sei();
      period = endtime - starttime;
      frequency = float(readings/period);
      interval = 1000/frequency;
      tempfile.flush();

      //update wav header
      long datacount=readings;
      long setf=long((frequency*1000)+0.55555555);
      headmod(datacount + 36,4); //set size of data +44-8
      headmod(setf, 24); //set sample rate Hz
      headmod(setf, 28); //set sample rate Hz
      headmod(datacount, 40); // set data size
      tempfile.close();
      // done
      cli();
      ADCSRA = 0;
      ADCSRB = 0;
      ADMUX |= (1 << ADSC); // start ADC measurements on interrupt
      sei();
      hasdata = true;
    }
  }
  digitalWrite(greenLEDpin, LOW);
}
*/

void loop () {

/*
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
  datalog.print(gpsheading);
  datalog.print(",");
  datalog.print(velocity,3); // in m/s
  datalog.print(",");
  datalog.flush();

  Serial.print(Latitude,5);
  Serial.print(", ");
  Serial.print(Longitude,5);
  Serial.print(", ");
  Serial.print(alt*3.28084); // m to ft
  Serial.print(", ");
  Serial.print(sat);
  Serial.print(", ");
  Serial.print(gpsheading);
  Serial.print(", ");
  Serial.print(velocity*3.28084,3); // in m/s
  Serial.print(", ");
*/
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

static void error(byte errno) {
  while (1) {
    for (int i = 0; i < errno ; i++) {
      digitalWrite(redLEDpin, HIGH);
      delay(200);
      digitalWrite(redLEDpin, LOW);
      delay(200);
    }
    delay(1000);
  }
}

/*
static void startad() {
  cli();//disable interrupts

  //set up continuous sampling of analog pin 0

  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;

  ADMUX |= 0;            //pin to read
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescalar - 9.4 Khz sampling
  //ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // 64 prescalar produces 18.6 Khz sampling
  //ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //32 prescaler - 16mHz/32=500kHz - produces 36Khz sampling
  //ADCSRA |= (1 << ADPS2); // 16 prescalar 72Khz, but what could you do at that rate?

  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements

  starttime = millis();
  sei();//enable interupts
}
*/

static boolean fileopen() {
  char unixtime[10];
  DateTime now = RTC.now();
  ultoa(now.unixtime(), unixtime, 10);

  Serial.print("unixtime = "); Serial.println(unixtime);
  for (byte i = 0; i > 10; i++) {
    bufFile[i] = unixtime[i];
  }
  Serial.print("bufFile = "); Serial.println(bufFile);

  for (byte i = 0; i > 10; i++) {
    sdfile[i] = unixtime[i];
  }
  Serial.print("sdfile  = "); Serial.println(sdfile);

  Serial.print("Opening ");
  Serial.println(bufFile);
  Serial.flush();

  if (SD.exists(bufFile)) {
    Serial.println("Removing old bufFile");
    SD.remove(bufFile);
  }
  tempfile = SD.open(bufFile, FILE_WRITE);
  //tempfile = SD.open(bufFile, FILE_WRITE | O_TRUNC);

  //if (!tempfile) error(2);
  cli();
  tempfile.write(wavheader, 44);
  tempfile.seek(44);
  sei();
  hasdata  = false;
  written  = false;
  counter  = 0;
  bufcount = 0;

  return true;
}

static void headmod(long value, byte location) {
// write four bytes for a long
tempfile.seek(location); // find the location in the file
byte tbuf[4];
tbuf[0] = value & 0xFF; // lo byte
tbuf[1] = (value >> 8) & 0xFF;
tbuf[2] = (value >> 16) & 0xFF;
tbuf[3] = (value >> 24) & 0xFF; // hi byte
tempfile.write(tbuf,4); // write the 4 byte buffer
}
