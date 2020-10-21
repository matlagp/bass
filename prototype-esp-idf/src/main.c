#include "main.h"

RingbufHandle_t buffer;

static char *received_server_ip;

void app_main()
{
  common_init();

  createMemoryDebugTask();

  buffer = createRingBuffer();

  createBluetoothTask(onWifiCredentialsReceived);
}

static void onWifiCredentialsReceived(char *ssid, char *password, char *server_ip)
{
  received_server_ip = server_ip;

  createWifiTask(ssid, password, onWifiNotConnected, onWifiConnected, onWifiDisconnected, onWifiReconnected);
}

static void onWifiNotConnected(void)
{
  retryBluetooth();
}

static void onWifiConnected(char *ip_address)
{
  cleanupBluetooth();

  vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for bluetooth cleanup

  #ifdef USE_WM8960
    wm8960_init();
    wm8960_set_vol(255);
  #endif
  init_i2s();
  init_ir();

  createUdpTask(buffer);
  createI2sTask(buffer);
  createMqttTask(ip_address, received_server_ip);
  createIrTask();
}

static void onWifiDisconnected(void)
{
  disconnectMqtt();
}

static void onWifiReconnected(char *ip_address)
{
  reconnectMqtt(ip_address);
}
