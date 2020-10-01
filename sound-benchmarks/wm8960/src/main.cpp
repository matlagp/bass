#include <Arduino.h>
#include <WiFi.h>
#include "wm8960.h"

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2SNoDAC.h"

#include "viola.h"

AudioGeneratorWAV *wav;
AudioFileSourcePROGMEM *file;
AudioOutputI2S *out;

void TaskI2C(void *pvParameters)
{
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  wm8960_set_vol(255);
  for (;;)
  {
    bool ret;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ret = out->SetOutputModeMono(true);
    Serial.printf("MONO: %d", ret);
    // rc = wm8960_set_mute(true);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    out->SetOutputModeMono(false);
    Serial.printf("STEREO: %d", ret);
    // rc = wm8960_set_mute(false);
  }
}

void setup()
{
  WiFi.mode(WIFI_OFF);
  Serial.begin(9600);
  wm8960_init();
  wm8960_set_vol(255);
  delay(1000);

  Serial.printf("WAV start\n");

  audioLogger = &Serial;
  file = new AudioFileSourcePROGMEM(viola, sizeof(viola));
  out = new AudioOutputI2S(I2S_NUM_0, 0, 32, 0);
  wav = new AudioGeneratorWAV();
  wav->begin(file, out);

  xTaskCreate(TaskI2C, "I2CTask", 2048, NULL, 2, NULL);
}

void loop()
{
  if (wav->isRunning())
  {
    if (!wav->loop())
    {
      wav->stop();
      file = new AudioFileSourcePROGMEM(viola, sizeof(viola));
      wav->begin(file, out);
    }
  }
  else
  {
    Serial.printf("WAV done\n");
    delay(1000);
  }
}

// void setup()
// {
//   Serial.begin(9600);

//   xTaskCreate(TaskI2C, "I2CTask", 2048, NULL, 2, NULL);
// }

// void loop()
// {
// }