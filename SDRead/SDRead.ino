#include <SD.h>
#define CSPIN 4
File myFile;
 
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
 
  // open the file for reading:
  String fileName = "test.txt";
  myFile = SD.open(fileName);
  if (myFile) {
    Serial.print(fileName);
    Serial.println(';');
 
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
 
void loop()
{
  // nothing happens after setup
}
