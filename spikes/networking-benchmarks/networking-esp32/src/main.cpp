#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

void connectToWiFi(const char *ssid, const char *pwd);
void WiFiEvent(WiFiEvent_t event);

const char *ssid = "...";
const char *pwd = "...";

WiFiUDP udp;
WiFiServer tcp;
WiFiClient client;
uint8_t rx[2048];
bool connected = false;

int data = 0;
unsigned long startTime = 0;
unsigned long endTime = 0;

void setup() {
  Serial.begin(9600);
  connectToWiFi(ssid, pwd);
}

void loop() {
  if (!connected) return;

  if (client.connected()) {
    if (client.available()) {
      data += client.read(rx, 2048);
    }
  } else {
    client = tcp.available();
  }

  int packetSize = udp.parsePacket();
  if (packetSize > 0)
  {
    data += udp.read(rx, 2048);
  }

  if ((endTime = millis()) - startTime >= 1000) {
    Serial.printf("%d\n", data);
    data = 0;
    startTime = endTime;
  }
}

void connectToWiFi(const char * ssid, const char * pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, pwd);
  Serial.println("Waiting for WIFI connection...");
}

void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          Serial.println("Multicast: 239.255.21.37");

          udp.beginMulticast(IPAddress(239,255,21,37), 2137);
          tcp.begin(2137);
          connected = true;
          startTime = millis();
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}