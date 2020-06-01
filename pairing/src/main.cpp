#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>

BluetoothSerial bluetoothSerial;

String ssid;
String password;
String serverIP;
int setupStep = 0;
int retries;

void setup() {
  Serial.begin(9600);

  bluetoothSerial.begin("Node-");

  uint32_t chipId = (uint32_t) ESP.getEfuseMac();
  Serial.printf("%u\n", chipId);
}

void loop() {
  if (bluetoothSerial.available()) {
    char type = bluetoothSerial.read();
    String data = bluetoothSerial.readStringUntil('\n');
    if (type == '1') {
      Serial.printf("SSID: ");
      ssid = data;
      setupStep = 1;
    } else if (setupStep == 1 && type == '2') {
      Serial.printf("PASS: ");
      password = data;
      setupStep = 2;
    } else if (setupStep == 2 && type == '3') {
      Serial.printf("IP:   ");
      serverIP = data;
      setupStep = 3;
    } else {
      Serial.printf("WHAT: ");
    }

    Serial.println(data);

    if (setupStep == 3) {
      Serial.println("Connecting to WiFi");
      bluetoothSerial.disconnect();

      WiFi.begin(ssid.c_str(), password.c_str());
      retries = 50;
    }
  }

  if (setupStep == 3) {
    if (WiFi.status() != WL_CONNECTED) {
      retries--;
    } else {
      Serial.println("Connected to WiFi");
      bluetoothSerial.end();
      setupStep = 4;
    }
  }

  if (setupStep == 3 && retries < 0) {
    Serial.println("Connecting to WiFi failed");
    setupStep = 0;
  }

  delay(50);
}