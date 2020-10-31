#include "wifi_task.h"

static void wifi_init(char *ssid, char *password);
static void wifi_event_handler(void *, esp_event_base_t, int32_t, void *);

static void (*on_not_connected_handler)(void);
static void (*on_connected_handler)(char *ip_address);
static void (*on_disconnected_handler)(void);
static void (*on_reconnected_handler)(char *ip_address);

static bool initialized = false;
static bool first_connection = true;
static char ip_address[INET_ADDRSTRLEN];

void createWifiTask(
    char *ssid,
    char *password,
    void (*on_not_connected)(void),
    void (*on_connected)(char *ip_address),
    void (*on_disconnected)(void),
    void (*on_reconnected)(char *ip_address))
{
  on_not_connected_handler = on_not_connected;
  on_connected_handler = on_connected;
  on_disconnected_handler = on_disconnected;
  on_reconnected_handler = on_reconnected;

  wifi_init(ssid, password);
}

static void wifi_init(char *ssid, char *password)
{
  if (!initialized)
  {
    initialized = true;
    ESP_ERROR_CHECK(esp_netif_init());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
  }

  wifi_config_t sta_config;
  bzero(&sta_config, sizeof(wifi_config_t));
  memcpy(sta_config.sta.ssid, ssid, min(32, strlen(ssid)));
  sta_config.sta.ssid[31] = '\0';
  memcpy(sta_config.sta.password, password, min(64, strlen(password)));
  sta_config.sta.password[63] = '\0';

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));

  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());
}

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    ESP_LOGI("wait", "WiFi disconnected");
    if (first_connection)
    {
      esp_wifi_stop();
      on_not_connected_handler();
    }
    else
    {
      on_disconnected_handler();
      esp_wifi_connect();
    }
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    const ip_event_got_ip_t *event = (const ip_event_got_ip_t *)event_data;
    snprintf(ip_address, INET_ADDRSTRLEN, IPSTR, IP2STR(&event->ip_info.ip));
    ESP_LOGI("wait", "Got IP: %s", ip_address);

    if (first_connection)
    {
      on_connected_handler(ip_address);
      first_connection = false;
    }
    else
    {
      on_reconnected_handler(ip_address);
    }
  }
}
