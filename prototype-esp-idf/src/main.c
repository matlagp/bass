#include "main.h"

#include "bluetooth_task.h"

RingbufHandle_t buffer;

void app_main()
{
  common_init();

  createMemoryDebugTask();

  createBluetoothTask();

  buffer = createRingBuffer();

  // createWifiTask(onWifiConnected, onWifiDisconnected, onWifiReconnected);

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

static void onWifiDisconnected(void)
{
  disconnectMqtt();
}

static void onWifiReconnected(char *ip_address)
{
  reconnectMqtt(ip_address);
}
