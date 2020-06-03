#ifndef ESP32
#define ESP32 1
#endif

#include <Arduino.h>
#include <Wire.h>
#include <FS.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SD.h>
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"
#include "WM8960.h"
#include "AudioBuffer.h"

AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;
WiFiUDP wifiudp;
AudioBuffer buffer = AudioBuffer(16);

const char *ssid = "...";
const char *pwd = "...";

bool connected = false;

char packetBuffer[128];

static void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  

          wifiudp.begin(2137);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}

void connectToWiFi(const char * ssid, const char * pwd) {
    Serial.println("Connecting to WiFi network: " + String(ssid));
    WiFi.disconnect(true);
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(ssid, pwd);
    Serial.println("Waiting for WIFI connection...");
}

void setup() {
    Serial.begin(9600);
    Wire.begin(I2C_SDA, I2C_SCL);

    byte wm_init_result = WM8960.begin();
    if (wm_init_result == 0) {
        Serial.printf("DAC initialized successfully\n");
    } else {
        Serial.printf("DAC initialization failed: %d\n", wm_init_result);
    }

    buffer.clear();

    connectToWiFi(ssid, pwd);

    audioLogger = &Serial;
    in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
    aac = new AudioGeneratorAAC();
    out = new AudioOutputI2S(0, AudioOutputI2S::EXTERNAL_I2S);
    out->SetPinout(I2S_CLK, I2S_WS, I2S_TXSDA);
    aac->begin(in, out);
}

void loop() {
    if (!connected) return;

    // if there's data available, read a packet
    int packetSize = wifiudp.parsePacket();
    if (packetSize) {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remoteIp = wifiudp.remoteIP();
        Serial.print(remoteIp);
        Serial.print(", port ");
        Serial.println(wifiudp.remotePort());

        int len = wifiudp.read(packetBuffer, 128);
        if (len > 0) {
            if (packetBuffer[0] == 'e') {
                int s = buffer.shift((byte*) packetBuffer, 10);
                Serial.printf("Read %d bytes\n", s);
            } else {
                int s = buffer.push((byte*) packetBuffer, len);
                Serial.printf("Wrote %d bytes\n", s);
            }
            buffer.debug();
        }
    }

    if (aac->isRunning()) {
        aac->loop();
    }
}