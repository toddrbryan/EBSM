#include <LoRa.h>
#include <TinyGPS.h>

#define DIO0 7
#define RESET 4
#define SS 8
#define VBATPIN A9
#define LORA_FREQ 9147E5
#define LORA_SF 12
#define LORA_BW 250E3
#define LORA_TX_POWER 20
#define LORA_CODING_RATE 8

#define CONSOLE 1

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

  Serial1.begin(9600);
  while( !Serial1 ) {
    delay(1);
  }
 
  delay(250);

  setup_LoRa();
  setup_gps();

  #ifdef CONSOLE
  Serial.println("32u4 II LoRa Data Node");
  #endif
  blink(5);
}

void setup_LoRa() {
  LoRa.setPins(SS, RESET, DIO0);
  LoRa.begin(LORA_FREQ);
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setTxPower(LORA_TX_POWER);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setCodingRate4(LORA_CODING_RATE);
}

void setup_gps() {
  Serial1.println("$PSRF105,01*3E"); //DEBUG ON
  Serial1.println("$PSRF104,34.4926,-119.8024,300,96000,167664,2098,12,08*30"); //full reset to factory defaults

  //Serial1.println("$PSRF105,0*3F"); //DEBUG OFF
//  Serial1.println("$PSRF103,02,00,00,01*26"); // GSA OFF
//  Serial1.println("$PSRF103,03,00,00,01*27"); //GSV OFF
//  Serial1.println("$PSRF103,04,00,00,01*20"); //RMC OFF
//  Serial1.println("$PSRF103,05,00,00,01*21"); //VTG OFF
//  Serial1.println("$PSRF103,00,00,00,01*24"); //GGA OFF
//  Serial1.println("$PSRF103,01,00,00,01*25"); //GLL OFF

  // Start 8 second delivery of RMC
  //Serial1.println("$PSRF103,04,00,8,01*21");
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
  read_battery_voltage();
}

int read_battery_voltage() {
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  //measuredvbat /= 1024; // convert to voltage
  #ifdef CONSOLE
  Serial.print("VBat: " ); Serial.println(measuredvbat); Serial.println((int)measuredvbat);
  #endif
  return 0;
}

void transmit() {
  byte *bptr = (byte *)&glon;

  buf[0] = bptr[3];
  buf[1] = bptr[2];
  buf[2] = bptr[1];
  bptr = (byte *)&glat;
  buf[3] = bptr[3];
  buf[4] = bptr[2];
  buf[5] = bptr[1];

  #ifdef CONSOLE
  Serial.println(glon);
  Serial.println(glat);
//  for(int i = 0; i< 4; i++) {
//    Serial.print(((byte *)&glon)[i], HEX);
//    Serial.print(" ");
//  }
//  Serial.println();
//  for(int i = 0; i< 6; i++) {
//    Serial.print(buf[i], HEX);
//    Serial.print(" ");
//  }
//  Serial.println();
  #endif
  LoRa.beginPacket();
  LoRa.write(buf, 6);
//  for(int i = 0; i< 6; i++) {
//    LoRa.write(buf[i]);
//  }
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
