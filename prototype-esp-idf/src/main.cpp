#include "main.h"
#include "rtp_jitter.h"

RTPJitter *buf;

extern "C" void app_main()
{
  common_init();

  createMemoryDebugTask();

  buf = new RTPJitter(200, 44100);

  // buffer = createRingBuffer();

  createWifiTask(onWifiConnected, onWifiDisconnected, onWifiReconnected);

  wm8960_init();
  wm8960_set_vol(255);

  init_i2s();
}

static void onWifiConnected(char *ip_address)
{
  createUdpTask(buf);
  // createI2sTask(buf);
  createMqttTask(ip_address);
}

static void onWifiDisconnected(void)
{
  disconnectMqtt();
}

static void onWifiReconnected(char *ip_address)
{
  reconnectMqtt(ip_address);
}
