#include "main.h"

#include "bluetooth_task.h"

RingbufHandle_t buffer;

void app_main()
{
  common_init();

  createMemoryDebugTask();

  buffer = createRingBuffer();

  createBluetoothTask(onWifiCredentialsReceived);
}

static void onWifiCredentialsReceived(void)
{
  printf("Received Wifi Credentials\r\n");

  createWifiTask(onWifiConnected, onWifiDisconnected, onWifiReconnected);
}

static void onWifiConnected(char *ip_address)
{
  cleanupBluetooth();

  vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for bluetooth cleanup

  wm8960_init();
  wm8960_set_vol(255);
  init_i2s();

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
