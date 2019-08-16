#include <WiFi.h>
#include <WiFiMulti.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
WiFiMulti WiFiMulti;

#define RED 0xF800
#define ORANGE 0xFA60
#define YELLOW 0xFFE0
#define GREEN 0x07E0
#define BLACK 0x0000
#define DEG2RAD 0.0174532925

boolean washingStatus = false;
int personIndex = 0;
int startTime;
int endTime;
int distances[500] = {0};
int distanceIndex = 0;

void setup() {
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Initializing...");
  Serial.begin(9600);
  WiFiMulti.addAP("helloWifi", "416Wifi!");
  Serial.println();
  Serial.println();
  Serial.println("Waiting for WiFi");

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected w/ IP:");
  Serial.println(WiFi.localIP());
  //Serial.println("Getting index position..."); These lines are for retrieving index position from server
  //String line = String();
  //sendData(-1, port, host, line);
  //char * writable = new char[line.length()+1];
  //line.toCharArray(writable, line.length());
  //writable[line.length()] = '\0';
  //sscanf(writable, "%l", personIndex);// -1 will never be sent in another scenario
  // delete[] writable;
  Serial.println("Collecting sensor data.");
}

void loop() {
  uint16_t port = 80;
  char * host = "192.168.1.132";
  //char * host = "172.20.10.2";
  delay(500);

startLoop:
  float volts = analogRead(34) * 0.0048828125; // value from sensor * (5/1024)
  double distance = 126.002 * pow(volts, -1);
  if (washingStatus == true) {
    int j = 0;
    while (distances[j] != 0 && distances[j] != NULL) {
      j++;
    }
    if (distance < 50) {
      distances[j] = distance;
    }
    //distanceIndex = j;
  }
  
  tft.fillScreen(TFT_BLACK);
  if (millis() - startTime < 5000) {
    testRED();
  } else if (millis() - startTime < 10000) {
    testORANGE();
  } else if (millis() - startTime < 15000) {
    testYELLOW();
  } else if (millis() - startTime <= 20000) {
    testGREEN();
  }

  if (distance < 35 && distance > 5) { //35 is distance from sensor to person's body/the counter. Data is sent
    // when distance > 35 - when the handwasher has left
    //5 is min val where sensor is accurate
    if (washingStatus == false) {
          startTime = millis();
    }
    if (washingStatus == true && distance > 25) { //hands moved out of sink (tinker with depending on sink)
      delay(2000); //frequency of distance checks functionally
      goto startLoop;
    } else if (distance < 26 && washingStatus == false) {
      washingStatus = true;
    }
  } else {
    if (washingStatus == true) {
      endTime = millis();
    }
    washingStatus = false;
    float timeSpent = (endTime - startTime) * pow(10, -3);
    if (timeSpent > 0) {
      String exchange = "";
      sendData(timeSpent, port, host, exchange, distances);
//      for (int g = distanceIndex; g < 500; g++) {
//        distances[g] = NULL;
//      }
      memset(distances,0,sizeof(distances));
      endTime = 0;
      startTime = 0;
      timeSpent = -1;
      personIndex++;
    }
  }
}

void sendData(float timeSpent, uint16_t port, char *host, String &output, int distances[]) {
  Serial.println();
  Serial.println("Connecting to server");
  delay(1000);
  WiFiClient client;
  int attempts = 0;
  int k = 0;
  while (distances[k] != 0 && distances[k] != NULL) {
    k++;
  }
  String message;
  if (timeSpent < 0) {
  } else {
    message += personIndex;
    message += ": ";
  }
  message += timeSpent;
  message += '\n';
  for (int i = 0; i < k; i++) {
    message += distances[i];
    message += ' ';

  }
  connectLine:
  attempts++;
  if (attempts > 2) {
    Serial.println("Failed");
    return;
  } else if (!client.connect(host, port)) {
    Serial.println("Failed");
    delay(5000);
    goto connectLine;
  }
  client.println(message);
  delay(25);
  Serial.println(message);
  if (client.connected()) {
    Serial.println(client.readStringUntil('\r'));
  }
  client.stop();
  return;
}

void testRED() {
  tft.fillCircle(64, 80, 64, RED); // face
  tft.fillCircle(92, 70, 14, BLACK); // left eye
  tft.fillCircle(37, 70, 14, BLACK); // right eye
      
  // Mouth (sad, curves down)
  float start_angle = 3.14, end_angle = 6.28;
  int x = 64, y = 130, r = 40;
  int vals[5] = {65, 45, 35, 28, 25};
  for(int ii = 2; ii <= 6; ii++){
    tft.fillCircle(64, 80, 64, RED); // face
    tft.fillCircle(92, 70, 14, BLACK); // left eye
    tft.fillCircle(37, 70, 14, BLACK); // right eye

    for (float i = start_angle; i < end_angle; i = i + 0.05)
      {
        tft.drawPixel(x + cos(i) * r, ((y + sin(i) * r)/ii) + (120 - vals[ii-2]), BLACK); // center point is (50,100)
        tft.drawPixel(x + cos(i) * r, ((y + sin(i) * r)/ii) + (120 - vals[ii-2]) + 1, BLACK); // center point is (50,100)
        }
    delay(500);
  }
}

void testORANGE() {
  tft.fillCircle(64, 80, 64, ORANGE); // face
  tft.fillCircle(92, 70, 14, BLACK); // left eye
  tft.fillCircle(37, 70, 14, BLACK); // right eye

  tft.fillRect(24, 113, 80, 2, BLACK);
}

void testYELLOW() {
  float start_angle = 0, end_angle = 3.14;
  int x = 64, y = 130, r = 40;
  int vals2[5] = {31, 32, 34, 37, 40};
  int count = 0;
  for(double ii = 6; ii >= 4; (ii = ii - 0.5)){
    tft.fillCircle(64, 80, 64, YELLOW); // face
    tft.fillCircle(92, 70, 14, BLACK); // left eye
    tft.fillCircle(37, 70, 14, BLACK); // right eye

    for (float i = start_angle; i < end_angle; i = i + 0.05)
      {
        tft.drawPixel(x + cos(i) * r, ((y + sin(i) * r)/ii) + (120 - vals2[count]), BLACK); // center point is (50,100)
        tft.drawPixel(x + cos(i) * r, ((y + sin(i) * r)/ii) + (120 - vals2[count]) + 1, BLACK); // center point is (50,100)
        }
    delay(500);
    count++;
  }
}

void testGREEN() {
  float start_angle = 0, end_angle = 3.14;
  int x = 64, y = 130, r = 40;
  int vals3[5] = {44, 51, 61, 76, 105};
  int count = 0;
  for(double ii = 3.5; ii >= 1.5; (ii = ii - 0.5)){
    tft.fillCircle(64, 80, 64, GREEN); // face
    tft.fillCircle(92, 70, 14, BLACK); // left eye
    tft.fillCircle(37, 70, 14, BLACK); // right eye

    for (float i = start_angle; i < end_angle; i = i + 0.05)
      {
        tft.drawPixel(x + cos(i) * r, ((y + sin(i) * r)/ii) + (120 - vals3[count]), BLACK); // center point is (50,100)
        tft.drawPixel(x + cos(i) * r, ((y + sin(i) * r)/ii) + (120 - vals3[count]) + 1, BLACK); // center point is (50,100)
        //tft.drawPixel(x + cos(i) * r, ((y + sin(i) * r)/ii) + (120 - vals[ii-2]) + 6, BLACK); // center point is (50,100)
        //tft.drawPixel(x + cos(i) * r, ((y + sin(i) * r)/ii) + (10 - vals[ii-2]) + 3, BLACK); // center point is (50,100)
      }
    delay(500);
    count++;
  }
}
