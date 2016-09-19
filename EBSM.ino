#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define RXPIN 13
#define TXPIN 8
#define ONE_WIRE_PIN 2

SoftwareSerial nss(RXPIN, TXPIN);
TinyGPS gps;
OneWire owb(ONE_WIRE_PIN);
DallasTemperature sensors(&owb);

char lineEnd = 0x0A;
bool timersSetUp = false;
int tempC[4] = {0, 0, 0, 0};
byte nTemps = 0;
long glat = 0;
long glon = 0;
unsigned long fix_age = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(RXPIN, INPUT);
  pinMode(TXPIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Started...");

  nss.begin(9600);
  // DEBUG ON nss.println("$PSRF105,01*3E");
  nss.println("$PSRF105,0*3F"); //DEBUG OFF
  nss.println("$PSRF103,02,00,00,01*26"); // GSA OFF
  nss.println("$PSRF103,03,00,00,01*27"); //GSV OFF
  nss.println("$PSRF103,04,00,00,01*20"); //RMC OFF
  nss.println("$PSRF103,05,00,00,01*21"); //VTG OFF
  nss.println("$PSRF103,00,00,00,01*24"); //GGA OFF
  nss.println("$PSRF103,01,00,00,01*25"); //GLL OFF

  // Start 1 second delivery of RMC, to be discontinued once we have a time fix
  nss.println("$PSRF103,04,00,01,01*21");

  sensors.begin();
  nTemps = sensors.getDeviceCount();
  Serial.print("One-wire Sensors found: "); 
  Serial.println(nTemps);
}

//
//byte nmeaChecksum(char str[]) {
//  byte checksum = 0; 
//  for(int i = 0; i < strlen(str); i++) { 
//    checksum = checksum ^ str[i];
//  }
//  return checksum;
//}

void loop() {

  if ((timeStatus() == timeSet) && !timersSetUp) {
    Serial.println("Setting up timers...");
    Alarm.timerRepeat(10,requestGpsInfo);
    Alarm.delay(2000);
    Alarm.timerRepeat(10,requestTemperatures);
    nss.println("$PSRF103,04,00,00,01*20");  // TURN off RMC delivery
    timersSetUp = true;
  }
  while (nss.available())
  {
    int c = nss.read();
    //Serial.print((char)c);
    if (gps.encode(c)) {
      processGps();
    }
  }

  Alarm.delay(0);
}


void requestGpsInfo() {
  //Serial.println("Requesting sentence from GPS");
  nss.println("$PSRF103,04,01,00,01*21"); // Request RMC sentence
}

void requestTemperatures() {
  //Serial.println("Requesting temperatures");
  sensors.requestTemperatures();
  Alarm.timerOnce(1, readTemperatures);
}

void readTemperatures() {
  bool changed = false;
  for(int i = 0; i < nTemps; i++ ) {
    int newT = sensors.getTempCByIndex(i);
    if(newT != tempC[i]) {
      tempC[i] = newT;
      changed = true;
    }
  }
  if (changed) {
    printStatus();
  }
}

void processGps() {
  long lat, lon;
  unsigned long time, date, speed, course;
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  int year;
  byte month, day, hour, minute, second, hundredths;
  bool changed = false;

  //Serial.println("Processing a sentence");

  gps.crack_datetime(&year, &month, &day,
  &hour, &minute, &second, &hundredths, &fix_age);
  if (fix_age < 500) {
    // set the Time to the latest GPS reading
    setTime(hour, minute, second, day, month, year);
  }
  gps.get_position(&lat, &lon, &fix_age);
  if(lat != glat) {
    glat = lat;
    changed = true;
  }
  if(lon != glon) {
    glon = lon;
    changed = true;
  }
  if(changed) {
    printStatus();
  }
}

void printStatus(){  
  printDigits(month(), false);
  Serial.print("/");
  printDigits(day(), false);
  Serial.print("/");
  Serial.print(year());

  Serial.print(" ");
  printDigits(hour(), false);
  printDigits(minute(), true);
  printDigits(second(), true);
  Serial.print("Z ");

  Serial.print(fix_age);
  Serial.print(" ");

  Serial.print(glat);
  Serial.print(" ");
  Serial.print(glon);

  for(int i = 0; i < nTemps; i++) {
    Serial.print(" ");
    Serial.print(tempC[i]);
    Serial.print("C");
  }
  Serial.println();
}

void printDigits(int digits, bool includeColon) {
  if(includeColon) {
    Serial.print(":");
  }
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

