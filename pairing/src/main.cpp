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

char topic_state[30];
char topic_settings[35];

int volume = 100;

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

void setup() {
  Serial.begin(9600);

  snprintf(name, 14, "node-%08X", (uint32_t) ESP.getEfuseMac());
  snprintf(topic_state, 30, "/nodes/%08X/state", (uint32_t) ESP.getEfuseMac());
  snprintf(topic_settings, 35, "/nodes/%08X/settings/#", (uint32_t) ESP.getEfuseMac());

  bluetoothSerial.begin(name);

  Serial.println(name);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  char msg_c_string[4] = "\0\0\0";

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();

  int j = 0;
  for (int i = 0; topic[i] != '\0'; i++) {
    if (topic[i] == '/') j = i+1;
  }

  if (strcmp(&topic[j], "volume") == 0) {
    strncpy(msg_c_string, (char*) message, length > 3 ? 3 : length);
    int newVolume = atoi(msg_c_string);
    if (newVolume >=0 && newVolume <= 100) {
      volume = newVolume;
      Serial.printf("Volume: %d\n", volume);
    } else {
      Serial.printf("Volume: %d is invalid\n", newVolume);
    }
  }
}

void loop() {
  // Reading WiFi config from Bluetooth
  if (setupStep < 3 && bluetoothSerial.available()) {
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
      retries = 200;
    }
  }

  // Connecting to WiFi
  if (setupStep == 3) {
    if (WiFi.status() != WL_CONNECTED) {
      retries--;
    } else {
      Serial.println("Connected to WiFi");
      Serial.println(WiFi.localIP());
      bluetoothSerial.end();
      setupStep = 4;

      pubSubClient.setServer(serverIP.c_str(), 1883);
      pubSubClient.setCallback(callback);
    }
  }

  // Back to BT
  if (setupStep == 3 && retries < 0) {
    Serial.println("Connecting to WiFi failed");
    setupStep = 0;
  }

  // Connecting to MQTT
  if (setupStep == 4
      && !pubSubClient.connected()
      && pubSubClient.connect(name, topic_state, 0, true, "0"))
  {
    Serial.println("Connected to MQTT");
    pubSubClient.subscribe(topic_settings);
    pubSubClient.publish(topic_state, WiFi.localIP().toString().c_str(), true);
    setupStep = 5;
  }

  // Handling messages and MQTT reconnecting
  if (setupStep == 5 && !pubSubClient.loop()) {
    setupStep = 4;
  }

  delay(50);
}