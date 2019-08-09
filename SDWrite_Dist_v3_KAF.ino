#include <SD.h>
#include <SPI.h>
#include <WiFi101.h>
#include "arduino_secrets.h"
#define sensor A0 // Sharp IR GP2Y0A41SK0F (4-30cm, analog)
#define CSPIN 4
#define piezo 3

//wifi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;
int status = WL_IDLE_STATUS;
WiFiServer server(80);

File myFile;
bool fileOpened;
bool washingStatus;
int washStart;
int washEnd;
int washTime;
int personCounter = 0;

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

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
    tone(piezo, 2000, 500);
  }

  Serial.print("Initializing Web Server...");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
  }
  Serial.print("Creating access point named: ");
  Serial.println(ssid);
  status = WiFi.beginAP(ssid);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
  }

  delay(10000);
  server.begin();
  printWiFiStatus();

  tone(piezo, 5000, 500);

}

void loop()
{
startLoop:


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

  if (status != WiFi.status()) {
    status = WiFi.status();
    if (status == WL_AP_CONNECTED) {
      byte remoteMac[6];
      WiFi.APClientMacAddress(remoteMac);
    } else {
      Serial.println("Device disconnected");
    }
  }

  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char d = client.read();
        Serial.write(d);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (d == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          while (myFile.available()) {
            client.print(myFile.read());
          }
          // close the file:
          myFile.close();
        }
        client.println("</html>");
        break;
      }
      if (d == '\n') {
        // you're starting a new line
        currentLineIsBlank = true;
      }
      else if (d != '\r') {
        // you've gotten a character on the current line
        currentLineIsBlank = false;
      }
    }
  }
  // give the web browser time to receive the data
  delay(1);

  // close the connection:
  client.stop();
  Serial.println("client disconnected");
}


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
    if ((washEnd - washStart) / 1000 >= 20) {
      tone(piezo, 2000, 250);
      delay(250);
      tone(piezo, 3250, 250);
      delay(250);
      tone(piezo, 4500, 250);
      delay(250);
      tone(piezo, 5750, 250);
    }
  }
  washingStatus = false;
}

washTime = washEnd - washStart;
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
    } else {

    }
    Serial.println("File writing done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
    tone(piezo, 2000, 500);
  }
}
}
