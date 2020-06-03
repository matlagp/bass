#ifndef ESP32
#define ESP32 1
#endif

#include <Arduino.h>
#include <Wire.h>
#include <FS.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <SD.h>
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"
#include "WM8960.h"

AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SDA, I2C_SCL);
  byte wm_init_result = WM8960.begin();

  if (wm_init_result == 0) {
    Serial.printf("DAC initialized successfully\n");
  } else {
    Serial.printf("DAC initialization failed: %d\n", wm_init_result);
  }

  audioLogger = &Serial;
  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
  aac = new AudioGeneratorAAC();
  out = new AudioOutputI2S(0, AudioOutputI2S::EXTERNAL_I2S);

  out->SetPinout(I2S_CLK, I2S_WS, I2S_TXSDA);

  aac->begin(in, out);
}

void loop() {
  if (aac->isRunning()) {
    aac->loop();
  } else {
    Serial.printf("AAC done\n");
    delay(1000);
  }
}