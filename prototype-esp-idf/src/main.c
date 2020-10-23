#include "main.h"

RingbufHandle_t buffer;

static char *received_server_ip;

static bool fresh_credentials;
static nvs_handle_t nvsHandle;
static const char *credentials_store = "credentials";
static const char *wifi_ssid_key = "wifi_ssid";
static const char *wifi_password_key = "wifi_passwd";
static const char *server_ip_key = "server_ip";
static char *memory_wifi_ssid;
static char *memory_wifi_password;
static char *memory_server_ip;

void app_main()
{
  common_init();

  createMemoryDebugTask();

  buffer = createRingBuffer();

  esp_err_t nvsErr = ERR_OK;
  size_t requiredSize = 0;

  ESP_ERROR_CHECK(nvs_open(credentials_store, NVS_READWRITE, &nvsHandle));
  nvsErr |= nvs_get_str(nvsHandle, wifi_ssid_key, NULL, &requiredSize);
  if (requiredSize > 0) {
    memory_wifi_ssid = (char *) malloc(requiredSize * sizeof(char));
    nvsErr |= nvs_get_str(nvsHandle, wifi_ssid_key, memory_wifi_ssid, &requiredSize);
  }
  nvsErr |= nvs_get_str(nvsHandle, wifi_password_key, NULL, &requiredSize);
  if (requiredSize > 0) {
    memory_wifi_password = (char *) malloc(requiredSize * sizeof(char));
    nvsErr |= nvs_get_str(nvsHandle, wifi_password_key, memory_wifi_password, &requiredSize);
  }
  nvsErr |= nvs_get_str(nvsHandle, server_ip_key, NULL, &requiredSize);
  if (requiredSize > 0) {
    memory_server_ip = (char *) malloc(requiredSize * sizeof(char));
    nvsErr |= nvs_get_str(nvsHandle, server_ip_key, memory_server_ip, &requiredSize);
  }

  switch (nvsErr) {
    case ERR_OK:
      fresh_credentials = false;
      ESP_LOGI("main", "SSID: %s \nPASS: %s \nSERVER IP: %s", memory_wifi_ssid, memory_wifi_password, memory_server_ip);
      onWifiCredentialsReceived(memory_wifi_ssid, memory_wifi_password, memory_server_ip);
      break;
    default:
      fresh_credentials = true;
      createBluetoothTask(onWifiCredentialsReceived);
  }
}

static void onWifiCredentialsReceived(char *ssid, char *password, char *server_ip)
{
  received_server_ip = server_ip;

  if (fresh_credentials) {
    nvs_set_str(nvsHandle, wifi_ssid_key, ssid);
    nvs_set_str(nvsHandle, wifi_password_key, password);
    nvs_set_str(nvsHandle, server_ip_key, server_ip);
  }

  createWifiTask(ssid, password, onWifiNotConnected, onWifiConnected, onWifiDisconnected, onWifiReconnected);
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
  setMqttServerUri(received_server_ip);

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
  setNodeIpAddress(ip_address);
  reconnectMqtt();
}
