#include <LoRa.h>
#include <TinyGPS.h>

#define DIO0 7
#define RESET 4
#define SS 8

#undef CONSOLE 1

TinyGPS gps;
unsigned long glat = 0;
unsigned long glon = 0;
unsigned long fix_age = 0;
int year;
byte month, day, hour, minute, second, hundredths;
byte buf[6];

void setup() {
#ifdef CONSOLE
  Serial.begin(9600);
  while (!Serial) {
    delay(1);
  }    
#endif

  int s1 = 0;
  Serial1.begin(9600);
  while( !Serial1 ) {
    delay(1);
    s1++;
    #ifdef CONSOLE
    if(s1 >= 50) {
      Serial.println("Could not initialize Serial1 (GPS)");
      exit(-1);
    }
    #endif
  }
 
  delay(250);

  LoRa.setPins(SS, RESET, DIO0);
  LoRa.begin(9147E5);
  LoRa.setSpreadingFactor(12);
  LoRa.setTxPower(20);
  LoRa.setSignalBandwidth(250E3);
  LoRa.setCodingRate4(8);

  Serial1.println("$PSRF105,01*3E"); //DEBUG ON

  //Serial.println((LoRa.begin(9147E5) == 1) ? "LoRa initialized on 914.7MHz" : "LoRa initialization error");
  Serial1.println("$PSRF104,34.4926,-119.8024,300,96000,167664,2098,12,08*30"); //full reset to factory defaults

  //Serial1.println("$PSRF105,0*3F"); //DEBUG OFF
//  Serial1.println("$PSRF103,02,00,00,01*26"); // GSA OFF
//  Serial1.println("$PSRF103,03,00,00,01*27"); //GSV OFF
//  Serial1.println("$PSRF103,04,00,00,01*20"); //RMC OFF
//  Serial1.println("$PSRF103,05,00,00,01*21"); //VTG OFF
//  Serial1.println("$PSRF103,00,00,00,01*24"); //GGA OFF
//  Serial1.println("$PSRF103,01,00,00,01*25"); //GLL OFF

  // Start 8 second delivery of RMC
  Serial1.println("$PSRF103,04,00,8,01*21");

  #ifdef CONSOLE
  Serial.println("32u4 II LoRa TX Location");
  #endif
  blink(5);
}

static bool smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  char buf;
  do 
  {
    while (Serial1.available()) {
      buf = Serial1.read();
      #ifdef CONSOLE
      //Serial.write(buf);
      #endif
      gps.encode(buf);
    }
  } while (millis() - start < ms);
  #ifdef CONSOLE
  Serial.println("Smartdelay timeout");
  #endif
}

void loop() {
  smartdelay(30000);
  processGps();
}

void transmit() {

  int imonth = month;
  int iday = day;
  int ihour = hour;
  int iminute = minute;
  int isecond = second;
  byte *bptr = (byte *)&glon;

  buf[0] = bptr[3];
  buf[1] = bptr[2];
  buf[2] = bptr[1];
  bptr = (byte *)&glat;
  buf[3] = bptr[3];
  buf[4] = bptr[2];
  buf[5] = bptr[1];


  
  
  //sprintf(buf, "%ld %ld %lu %d %d %d %d %d %d", glat, glon, fix_age, year, imonth, iday, ihour, iminute, isecond);
  #ifdef CONSOLE
   Serial.println(glon);
    Serial.println(glat);
  for(int i = 0; i< 4; i++) {
    Serial.print(((byte *)&glon)[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  for(int i = 0; i< 6; i++) {
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  #endif
  LoRa.beginPacket();
  for(int i = 0; i< 6; i++) {
    LoRa.write(buf[i]);
  }
  LoRa.endPacket();
  #ifdef CONSOLE
  Serial.println("Sent LoRa packet");
  #endif
  blink(2);
}

void processGps() {
  long lat, lon;
  unsigned long time, date, speed, course;
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  bool changed = false;

  //Serial.println("Processing a sentence");
  
//  gps.crack_datetime(&year, &month, &day,
//  &hour, &minute, &second, &hundredths, &fix_age);
  gps.get_position(&lat, &lon, &fix_age);
  blink(3);
  glat = lat;
  glon = abs(lon);
  transmit();
}


void blink(int n) {
  for(int i = 0; i<n;i++) {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(200);              // wait for a second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    delay(200);
  }
}
