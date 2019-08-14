#include <WiFi.h>
#include <WiFiMulti.h>
WiFiMulti WiFiMulti;

boolean washingStatus = false;
int personIndex;
int startTime;
int endTime;
int distances[500] = {0};

void setup() {
  Serial.begin(9600);
  WiFiMulti.addAP("Kevin's iPhone", "myNameJeff");
  uint16_t port = 80;
  //char * host = "192.168.1.132";
  char * host = "172.20.10.2";
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
  //char * host = "192.168.1.132";
  char * host = "172.20.10.2";
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
      memset(distances, 0, sizeof(distances));
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
  client.println(message);
  delay(25);
  Serial.println(message);
  output = client.readStringUntil('\r');
  Serial.println("Server: " + output + '\n');
  client.stop();
  return;
}

