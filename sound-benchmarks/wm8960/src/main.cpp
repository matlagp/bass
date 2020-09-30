#include <Arduino.h>
#include <FreeRTOS.h>
#include <wm8960.h>
#include <sine.h>

void TaskI2C(void *pvParameters)
{
  wm8960_init();
  // init_i2s();
  // setup_triangle_sine_waves(16);

  for (;;)
  {
    wm8960_set_vol(255);
    esp_err_t rc;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    rc = wm8960_set_mute(true);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    rc = wm8960_set_mute(false);
  }
}

void setup()
{
  Serial.begin(9600);

  xTaskCreate(TaskI2C, "I2CTask", 2048, NULL, 2, NULL);
}

void loop()
{
}