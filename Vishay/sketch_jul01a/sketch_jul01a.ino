#include "CRC_VCNL4200.h"
#include "Wire.h"
CRC_VCNL4200 vcnl4200;
int pinReading = 0;
uint16_t dist = 0;
void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Booting.");
  if (vcnl4200.exists()) {
    Serial.println("VCNL4200 found");
    vcnl4200.initialize();
    Serial.println("VCNL4200 initialized");
  }
  //pinMode(7, INPUT);
  pinMode(13, OUTPUT);
}

void loop() {
  
  Serial.print("Proximity: ");
  Serial.println(vcnl4200.getProximity());
  Serial.print("Ambient: ");
  Serial.println(vcnl4200.getAmbient());
  dist = vcnl4200.getProximity();
  double dis = dist*1.0; //Will be representing distance in meters.
  //30-45 deg veiwing angle, dist is reliable up to 1m about
  if(dist>280)
  {
    dis = (log(dis)-22.1)/-54;
  }
  else if(dist>20)
  {
    dis = (log(dis)-6.4)/-2.5;
  }
  else
  {
    dis = 1.5;
  }

  //The onboard LED will light up if an object is detected between .75 and 1 meter.
  if(.75>dis>1.00)
  {
    digitalWrite(13, HIGH);
  }
  else
  {
    digitalWrite(13,LOW);
  }
  Serial.print("Dist: ");
  Serial.println(dist);
  Serial.println(dis);

  //Check interrupt pin
  //pinReading = digitalRead(7);
  //Set LED to interrupt pin HIGH/LOW
  //Serial.print("High interrupt: ");
  //Serial.println(vcnl4200.getProxHighInterrupt());
  //Serial.print("Low interrupt: ");
  //Serial.println(vcnl4200.getProxLowInterrupt());
/*
  if (pinReading == HIGH) {
    //Serial.println("Interrupt false");
    digitalWrite(13, LOW);
  }
  else {
    //Serial.println("Interrupt true, flag:");
    //Serial.print(vcnl4200.getInterruptFlag());
    digitalWrite(13, HIGH);
  }
*/
  Serial.println();
  delay(1000);
}
