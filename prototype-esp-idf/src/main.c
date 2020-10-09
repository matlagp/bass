#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/ringbuf.h>
#include <esp_wifi.h>
#include <soc/efuse_reg.h>
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "driver/i2s.h"
#include "mqtt_client.h"

#include "wm8960.h"
#include "i2s_setup.h"

RingbufHandle_t buffer;

static EventGroupHandle_t s_wifi_event_group;
static const int CONNECTED_BIT = BIT0;

void memory_report()
{
  ESP_LOGI("mem", "heap: %d", xPortGetFreeHeapSize());
  ESP_LOGI("mem", "mheap: %d", xPortGetMinimumEverFreeHeapSize());
  ESP_LOGI("mem", "bheap: %u", heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
}

void memTask(void *params)
{
  for (;;)
  {
    memory_report();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void udpTask(void *params)
{
  char rx[500];

  for (;;)
  {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(2137);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
      ESP_LOGE("udp", "Unable to create socket: errno %d", errno);
      break;
    }
    ESP_LOGI("udp", "Socket created");

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
      ESP_LOGE("udp", "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI("udp", "Socket bound, port 2137");

    for (;;)
    {
      ESP_LOGI("udp", "Waiting for data");
      int len = recvfrom(sock, rx, 500, 0, NULL, NULL);

      if (len < 0)
      {
        ESP_LOGE("udp", "recvfrom failed: errno %d", errno);
        break;
      }
      else
      {
        ESP_LOGI("udp", "recvfrom success");
        xRingbufferSend(buffer, rx, len, 100);
      }
    }
  }
  vTaskDelete(NULL);
}

void i2sTask(void *params)
{
  size_t bytes_written;
  uint8_t tx_blank[500];
  memset(tx_blank, 0, 500);

  for (;;)
  {
    uint8_t *tx = (uint8_t *)xRingbufferReceive(buffer, &bytes_written, 10);
    if (tx == NULL)
    {
      i2s_write(I2S_NUM_0, tx_blank, 500, &bytes_written, portMAX_DELAY);
      continue;
    }
    i2s_write(I2S_NUM_0, tx, 500, &bytes_written, portMAX_DELAY);

    vRingbufferReturnItem(buffer, tx);
  }
}

esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED:
  {
    char *topic_state = malloc(30);
    char *topic_settings = malloc(35);

    uint64_t chipmacid;
    ESP_ERROR_CHECK(esp_efuse_mac_get_default((uint8_t *)(&chipmacid)));

    snprintf(topic_state, 30, "/nodes/%08X/state", (uint32_t)(chipmacid >> 16));
    snprintf(topic_settings, 35, "/nodes/%08X/settings/#", (uint32_t)(chipmacid >> 16));

    ESP_LOGI("mqtt", "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_publish(client, topic_state, "192.168.11.117", 0, 1, 1);
    ESP_LOGI("mqtt", "sent publish successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(client, topic_settings, 1);
    ESP_LOGI("mqtt", "sent subscribe successful, msg_id=%d", msg_id);
    break;
  }
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI("mqtt", "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI("mqtt", "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    ESP_LOGI("mqtt", "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI("mqtt", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI("mqtt", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI("mqtt", "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI("mqtt", "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI("mqtt", "Other event id:%d", event->event_id);
    break;
  }
  return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD("mqtt", "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  mqtt_event_handler_cb(event_data);
}

void mqttTask(void *params)
{
  char *name = malloc(14);
  char *topic_state = malloc(30);
  char *topic_settings = malloc(35);

  uint64_t chipmacid;
  ESP_ERROR_CHECK(esp_efuse_mac_get_default((uint8_t *)(&chipmacid)));

  snprintf(name, 14, "node-%08X", (uint32_t)(chipmacid >> 16));
  snprintf(topic_state, 30, "/nodes/%08X/state", (uint32_t)(chipmacid >> 16));
  snprintf(topic_settings, 35, "/nodes/%08X/settings/#", (uint32_t)(chipmacid >> 16));

  ESP_LOGE("mqtt", "Registering as %s", name);

  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = "mqtt://192.168.11.113",
      .port = 1883,
      .lwt_topic = topic_state,
      .lwt_qos = 0,
      .lwt_msg = "0",
      .lwt_retain = true};
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client));

  ESP_ERROR_CHECK(esp_mqtt_client_start(client));

  ESP_LOGI("mqtt", "Mqtt connection started");
  vTaskDelete(NULL);
}

void waitingTask(void *params)
{
  EventBits_t uxBits;

  for (;;)
  {
    ESP_LOGI("wait", "WiFi Connecting");
    uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, true, false, portMAX_DELAY);
    if (uxBits & CONNECTED_BIT)
    {
      ESP_LOGI("wait", "WiFi Connected to ap");
      TaskHandle_t xHandle = NULL;
      xTaskCreate(udpTask, "udp_server", 4096, NULL, 5, &xHandle);
      if (xHandle == NULL)
      {
        ESP_LOGE("udp", "Could not create task");
        abort();
      }
      xTaskCreate(i2sTask, "i2s_task", 4096, NULL, 5, &xHandle);
      if (xHandle == NULL)
      {
        ESP_LOGE("i2s", "Could not create task");
        abort();
      }
      xTaskCreate(mqttTask, "mqtt_task", 4096, NULL, 5, &xHandle);
      if (xHandle == NULL)
      {
        ESP_LOGE("mqtt", "Could not create task");
        abort();
      }
      vTaskDelete(NULL);
    }
  }
}
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    TaskHandle_t xHandle = NULL;
    xTaskCreate(waitingTask, "waitingTask", 4096, NULL, 3, &xHandle);
    if (xHandle == NULL)
    {
      ESP_LOGE("wait", "Could not create task");
      abort();
    }
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    esp_wifi_connect();
    xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
  }
}

void app_main()
{
  xTaskCreate(memTask, "memTask", 4096, NULL, 5, NULL);
  buffer = xRingbufferCreate(500 * 260, RINGBUF_TYPE_BYTEBUF);
  if (buffer == NULL)
  {
    ESP_LOGE("buffer", "NULL");
    abort();
  }
  else
  {
    ESP_LOGI("buffer", "%u", xRingbufferGetCurFreeSize(buffer));
  }

  ESP_ERROR_CHECK(nvs_flash_init());

  ESP_ERROR_CHECK(esp_netif_init());
  s_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  wifi_config_t sta_config = {
      .sta = {
          .ssid = "...",
          .password = "..."},
  };
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());

  wm8960_init();
  wm8960_set_vol(255);

  init_i2s();
}
