#include <SD.h>
#define sensor A0 // Sharp IR GP2Y0A41SK0F (4-30cm, analog)
#define CSPIN 4
#define piezo 3

File myFile;
bool fileOpened;
bool washingStatus;
int washStart;
int washEnd;
int personCounter = 0;

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
    tone(piezo,2000, 500);
  }

  tone(piezo, 5000, 500);

}

void loop()
{
startLoop:
  float volts = analogRead(sensor) * 0.0048828125; // value from sensor * (5/1024)
  double distance = 26.4 * pow(volts, -1); // worked out from datasheet graph
  delay(750); // slow down serial port

  if (distance < 35 && distance > 10) { //Should change vals depending on sink
    Serial.println("Handwasher Detected.");
    if (washingStatus == true && distance > 25) {
      delay(5000);
      goto startLoop;
    } else if (distance < 26 && washingStatus == false) {
      Serial.println("Washing commenced.");
      washingStatus = true;
      tone(piezo, 3000, 500);
      washStart = millis();
      personCounter++;
    }
  } else {
    if (washingStatus == true) {
      washEnd = millis();
    }
    washingStatus = false;
  }

  int washTime = washEnd - washStart;
  Serial.print("#");
  Serial.print(personCounter);
  Serial.print(": ");
  Serial.print(volts);
  Serial.print(" V ");
  Serial.print('\t');
  Serial.print(distance);
  Serial.print(" cm");
  Serial.print('\t');
  if ((washTime / 1000) > 0) {
    Serial.print(washTime / 1000);
    Serial.print(" sec. spent washing. ");
  }


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
      myFile.print(personCounter);
      myFile.print('\t');
      myFile.print(volts);
      myFile.print('\t');
      myFile.print(distance);
      myFile.print('\t');
      if ((washTime / 1000) > 0) {
        myFile.print(washTime / 1000);
      }
      Serial.println("File writing done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
      tone(piezo,2000, 500);
    }
  }
}
