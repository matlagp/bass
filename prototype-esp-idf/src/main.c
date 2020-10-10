#include "main.h"

RingbufHandle_t buffer;

void app_main()
{
  createMemoryDebugTask();

  buffer = createRingBuffer();

  createWifiTask(onWifiConnected);

  wm8960_init();
  wm8960_set_vol(255);

  init_i2s();
}

static void onWifiConnected(char *ip_address)
{
  createUdpTask(buffer);
  createI2sTask(buffer);
  createMqttTask(ip_address);
}
