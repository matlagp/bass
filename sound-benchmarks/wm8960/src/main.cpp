#include <Arduino.h>
#include <FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "wm8960.h"
#include "sine.h"

void connectToWiFi(const char *ssid, const char *pwd);
void WiFiEvent(WiFiEvent_t event);

void receiveTask(void *);
void i2sTask(void *);

const char *ssid = "...";
const char *pwd = "...";

WiFiUDP udp;
uint8_t rx[500];
bool connected = false;

RingbufHandle_t buffer;

void setup()
{
  Serial.begin(9600);
  wm8960_init();
  wm8960_set_vol(255);
  connectToWiFi(ssid, pwd);

  init_i2s();

  buffer = xRingbufferCreate(100000, RINGBUF_TYPE_NOSPLIT);
  if (buffer == NULL)
  {
    Serial.println("Failed to create ring buffer");
  }

  xTaskCreate(
      receiveTask,
      "receiveTask",
      (1 << 13),
      NULL,
      2,
      NULL);

  xTaskCreate(
      i2sTask,
      "i2sTask",
      1024,
      NULL,
      2,
      NULL);
}

void loop()
{
}

void connectToWiFi(const char *ssid, const char *pwd)
{
  Serial.println("Connecting to WiFi network: " + String(ssid));
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, pwd);
  Serial.println("Waiting for WIFI connection...");
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.print("WiFi connected! IP address: ");
    Serial.println(WiFi.localIP());

    udp.begin(2137);
    connected = true;

    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("WiFi lost connection");
    connected = false;
    break;
  default:
    break;
  }
}

void receiveTask(void *pvParameters)
{
  for (;;)
  {
    if (!WiFi.isConnected())
    {
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
  for (;;)
  {
    uint8_t *tx = (uint8_t *)xRingbufferReceive(buffer, &bytes_written, portMAX_DELAY);
    if (tx == NULL)
    {
      Serial.println("Failed to receive item");
      continue;
    }

    for (int i = 0; i < 5; i++)
    {
      i2s_write(I2S_NUM_0, tx + 100 * i, 100, &bytes_written, portMAX_DELAY);
    }

    vRingbufferReturnItem(buffer, tx);
  }
}
