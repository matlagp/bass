#include "persistent_credentials.h"

static const char *credentials_store = "credentials";
static const char *wifi_ssid_key = "wifi_ssid";
static const char *wifi_password_key = "wifi_passwd";
static const char *server_ip_key = "server_ip";

static nvs_handle_t nvsHandle;

static esp_err_t loadCredential(const char *, char **);

esp_err_t loadPersistedCredentials() {
  ESP_ERROR_CHECK(nvs_open(credentials_store, NVS_READWRITE, &nvsHandle));

  esp_err_t nvsErr = loadCredential(wifi_ssid_key, &memory_wifi_ssid);
  nvsErr |= loadCredential(wifi_password_key, &memory_wifi_password);
  nvsErr |= loadCredential(server_ip_key, &memory_server_ip);

  if (nvsErr == ESP_OK) {
    ESP_LOGI(CREDENTIALS_TAG, "Loaded credentials from NVS:");
    ESP_LOGI(CREDENTIALS_TAG, "\tSSID: %s", memory_wifi_ssid);
    ESP_LOGI(CREDENTIALS_TAG, "\tPASSWORD: %s", memory_wifi_password);
    ESP_LOGI(CREDENTIALS_TAG, "\tSERVER IP: %s", memory_server_ip);
  } else {
    ESP_LOGE(CREDENTIALS_TAG, "Error while loading creentials (code %d)", nvsErr);
  }

  return nvsErr;
}

static esp_err_t loadCredential(const char *key, char **value) {
  size_t requiredSize = 0;
  esp_err_t loadErr = ESP_OK;

  loadErr |= nvs_get_str(nvsHandle, key, NULL, &requiredSize);
  if (loadErr == ESP_OK && requiredSize > 0) {
    *value = (char *) malloc(requiredSize * sizeof(char));
    loadErr |= nvs_get_str(nvsHandle, key, *value, &requiredSize);
  }

  return loadErr;
}

esp_err_t saveCredentials(const char *ssid, const char *password, const char *server_ip) {
  esp_err_t nvsErr = nvs_set_str(nvsHandle, wifi_ssid_key, ssid);
  nvsErr |= nvs_set_str(nvsHandle, wifi_password_key, password);
  nvsErr |= nvs_set_str(nvsHandle, server_ip_key, server_ip);

  if (nvsErr == ESP_OK) {
    ESP_LOGI(CREDENTIALS_TAG, "Saved new credentials");
  } else {
    ESP_LOGE(CREDENTIALS_TAG, "Error while saving credentials (code %d)", nvsErr);
  }

  return nvsErr;
}

void freeCredentialsMemory() {
  free(memory_wifi_ssid);
  free(memory_wifi_password);
  free(memory_server_ip);
}
