#include <SD.h>
#define sensor A0 // Sharp IR GP2Y0A41SK0F (4-30cm, analog)
#define CSPIN 4

File myFile;
bool fileOpened;
void setup()
{
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(CSPIN, OUTPUT);

  if (!SD.begin(CSPIN)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    fileOpened = true;
  } else {
    fileOpened = false;
    Serial.println("error opening test.txt in setup stage");
  }

}

void loop()
{
  // nothing happens after setup
  float volts = analogRead(sensor) * 0.0048828125; // value from sensor * (5/1024)
  double distance = 26.4 * pow(volts, -1); // worked out from datasheet graph
  delay(750); // slow down serial port
  Serial.print(volts);
  Serial.print('\t');
  Serial.print(distance);
  Serial.println(" cm");

  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'c') {
      if (fileOpened) {
        myFile.close();
      } else {
        myFile = SD.open("test.txt", FILE_WRITE);
      }
      fileOpened = !fileOpened;
    }
  }

  if (fileOpened) {
    if (myFile) {
      myFile.print(volts);
      myFile.print('\t');
      myFile.print(distance);
      myFile.println(" cm");
      Serial.println("writing done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  }


}
