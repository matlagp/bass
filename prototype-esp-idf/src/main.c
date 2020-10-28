#include "main.h"

RingbufHandle_t buffer;

static bool fresh_credentials;

void app_main()
{
  common_init();

  createMemoryDebugTask();

  buffer = createRingBuffer();

  switch (loadPersistedCredentials()) {
    case ESP_OK:
      fresh_credentials = false;
      onWifiCredentialsReceived(memory_wifi_ssid, memory_wifi_password, memory_server_ip);
      break;
    default:
      fresh_credentials = true;
      createBluetoothTask(onWifiCredentialsReceived);
  }
}

static void onWifiCredentialsReceived(char *ssid, char *password, char *server_ip)
{
  setMqttServerUri(server_ip);

  createWifiTask(ssid, password, onWifiNotConnected, onWifiConnected, onWifiDisconnected, onWifiReconnected);

  if (fresh_credentials) {
    saveCredentials(ssid, password, server_ip);
  } else {
    freeCredentialsMemory();
  }
}

static void onWifiNotConnected(void)
{
  fresh_credentials = true;
  retryBluetooth();
}

static void onWifiConnected(char *ip_address)
{
  cleanupBluetooth();

  vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for bluetooth cleanup

  setNodeId();
  setNodeIpAddress(ip_address);

  #ifdef USE_WM8960
    wm8960_init();
    wm8960_set_vol(255);
  #endif
  init_i2s();
  init_ir();

  createUdpTask(buffer);
  createI2sTask(buffer);
  createMqttTask();
  createIrTask();
}

static void onWifiDisconnected(void)
{
  disconnectMqtt();
}

static void onWifiReconnected(char *ip_address)
{
  setNodeIpAddress(ip_address);
  reconnectMqtt();
}
