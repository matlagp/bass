#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "wm8960.h"
#include "sine.h"

void connectToWiFi(const char *ssid, const char *pwd);
void WiFiEvent(WiFiEvent_t event);

const char *ssid = "...";
const char *pwd = "...";

WiFiUDP udp;
uint8_t rx[500];
bool connected = false;

void setup()
{
  Serial.begin(9600);
  wm8960_init();
  wm8960_set_vol(255);
  connectToWiFi(ssid, pwd);

  init_i2s();
}

void loop()
{
  if (!connected)
    return;

  if (udp.parsePacket() > 0)
  {
    size_t bytes_written;
    udp.read(rx, 500);
    for (int i = 0; i < 5; i++)
    {
      i2s_write(I2S_NUM_0, rx + 100 * i, 100, &bytes_written, 100);
    }
  }
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
