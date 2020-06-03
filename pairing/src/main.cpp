#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>

BluetoothSerial bluetoothSerial;

String ssid;
String password;
String serverIP;
int setupStep = 0; // TODO: turn into an enum
int retries;

char name[14];

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

void setup() {
  Serial.begin(9600);

  snprintf(name, 14, "Node-%08X", (uint32_t) ESP.getEfuseMac());

  bluetoothSerial.begin(name);

  Serial.println(name);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
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

      pubSubClient.setServer(serverIP.c_str(), 1883);
      pubSubClient.setCallback(callback);
    }
  }

  if (setupStep == 3 && retries < 0) {
    Serial.println("Connecting to WiFi failed");
    setupStep = 0;
  }

  if (setupStep == 4) {
    if (!pubSubClient.connected()) {
      pubSubClient.connect("Node-");
    } else {
      Serial.println("Connected to MQTT");
      pubSubClient.subscribe("house/bulb1");
      setupStep = 5;
    }
  }

  delay(50);
}