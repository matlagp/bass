#include <Arduino.h>
#include <WiFi.h>
#include "ServerConnection.h"

#include "FreeRTOS.h"
#include "wm8960.h"
#include "i2s_setup.h"
#include "stretch.h"
#include "driver/i2s.h"
#include "freertos/ringbuf.h"

WiFiUDP udp;
uint8_t rx[500];

RingbufHandle_t buffer;

void receiveTask(void *);
void i2sTask(void *);

void setup()
{
  Serial.begin(9600);

  wm8960_init();
  wm8960_set_vol(255);

  ServerConnection::loop_forever();

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Not connected yet");
    delay(1000);
  }
  Serial.println("Connected");

  delay(1000);

  // Sometimes crashes on startup, my guess is that his is too big
  for (;;)
  {
    buffer = xRingbufferCreateNoSplit(500, 35);
    if (buffer != NULL)
      break;
    Serial.println("Failed to create ring buffer");
    delay(1000);
  }

  xTaskCreate(
      receiveTask,
      "receiveTask",
      (1 << 13),
      NULL,
      3,
      NULL);

  xTaskCreate(
      i2sTask,
      "i2sTask",
      (1 << 13),
      NULL,
      3,
      NULL);
}

void loop()
{
}

void receiveTask(void *pvParameters)
{

  udp.begin(2137);

  for (;;)
  {
    if (!WiFi.isConnected())
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }

    if (udp.parsePacket() > 0)
    {
      udp.read(rx, 500);
      xRingbufferSend(buffer, rx, 500, 100);
    }
  }
}

void i2sTask(void *pvParameters)
{
  size_t bytes_written;
  uint8_t tx_blank[500];
  memset(tx_blank, 0, 500);

  for (;;)
  {
    uint8_t *tx = (uint8_t *)xRingbufferReceive(buffer, &bytes_written, 10);
    if (tx == NULL)
    {
      i2s_write(I2S_NUM_0, tx_blank, 500, &bytes_written, portMAX_DELAY);
      continue;
    }
    i2s_write(I2S_NUM_0, tx, 500, &bytes_written, portMAX_DELAY);

    vRingbufferReturnItem(buffer, tx);
  }
}
