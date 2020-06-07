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
#include <AsyncUDP.h>
#include <SD.h>
#include "AudioGeneratorAAC.h"
#include "AudioGeneratorWAV.h"
#include "AudioGeneratorRaw.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "casio.h"
#include "WM8960.h"
#include "CircularBuffer.h"
#include "AudioFileSourceRAM.h"

// AudioFileSourcePROGMEM *in;
AudioFileSourceRAM *in;
AudioGeneratorRaw *aac;
AudioOutputI2S *out;
AsyncUDP wifiudp;
CircularBuffer buffer = CircularBuffer(65565);

const char *ssid = "...";
const char *pwd = "...";

bool connected = false;

static void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.print("WiFi connected! IP address: ");
            Serial.println(WiFi.localIP());  


            if (wifiudp.listen(2137)) {
                Serial.println("UDP server up");
                wifiudp.onPacket([](AsyncUDPPacket packet) {
                    Serial.print("Received packet of size ");
                    Serial.println(packet.length());
                    Serial.print("From ");
                    IPAddress remoteIp = packet.remoteIP();
                    Serial.print(remoteIp);
                    Serial.print(", port ");
                    Serial.println(packet.remotePort());

                    if (packet.length() > 0) {
                        int s = buffer.push(packet.data(), packet.length());
                        Serial.printf("Wrote %d bytes\n", s);
                        // buffer.debug();
                    }
                });
            }
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

void setupAudioChain() {
    audioLogger = &Serial;

    in = new AudioFileSourceRAM(&buffer);
    aac = new AudioGeneratorRaw();
    // in = new AudioFileSourcePROGMEM(casio, sizeof(casio));
    // aac = new AudioGeneratorWAV();
    out = new AudioOutputI2S(0, AudioOutputI2S::EXTERNAL_I2S, 8, 1);

    out->SetBitsPerSample(16);
    out->SetChannels(2);
    out->SetRate(44100);
    out->SetOutputModeMono(false);
    out->SetPinout(I2S_CLK, I2S_WS, I2S_TXSDA);

    aac->begin(in, out);
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
    setupAudioChain();
}

void loop() {
    if (!connected) return;

    if (aac->isRunning()) {
        aac->loop();
    }
}