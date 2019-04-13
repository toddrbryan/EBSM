#include <SoftwareSerial.h>
#include <TimeAlarms.h>
#include <util/crc16.h>


const int trigPin = 7;
const int echoPin = 6;
long duration;
int distance;
const char *loc = "cabin31";
const char *site = "north_tanks";

SoftwareSerial HC12(10, 11); // HC-12 TX Pin, HC-12 RX Pin

void setup() {
  Serial.begin(9600);             // Serial port to computer
  HC12.begin(9600);               // Serial port to HC12
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

}

void loop() {
//  while (HC12.available()) {        // If HC-12 has data
//    Serial.write(HC12.read());      // Send the data to Serial monitor
//  }
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration*0.0343/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Duration: ");
  Serial.println(duration);
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print("cm (");
  Serial.print(distance/2.54);
  Serial.println("in)");
  Alarm.delay(2000);
  sendTankLevel(duration);
  
//  while (Serial.available()) {      // If Serial monitor has data
//    HC12.write(Serial.read());      // Send that data to HC-12
//  }
}

void sendTankLevel(long level) {
  char msg[32];
  char crc[16];
  sprintf(msg, "%s,%s,%d", loc, site, level);
  sprintf(crc, ",%d", calcCRC(msg));
  strcat(msg, crc);
  HC12.write(msg);
}


uint16_t calcCRC(char* str)
{
  uint16_t crc=0; // starting value as you like, must be the same before each calculation
  for (int i=0;i<strlen(str);i++) // for each character in the string
  {
    crc= _crc16_update (crc, str[i]); // update the crc value
  }
  return crc;
}

/*
 * uint16_t crc16_update(uint16_t crc, uint8_t a)
{
  int i;
  crc ^= a;
  for (i = 0; i < 8; ++i)
  {
    if (crc & 1)
    crc = (crc >> 1) ^ 0xA001;
    else
    crc = (crc >> 1);
  }
  return crc;
}*/

 

