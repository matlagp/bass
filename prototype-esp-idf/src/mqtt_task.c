#include "mqtt_task.h"

static void mqttTask(void *);
static void mqtt_event_handler(void *, esp_event_base_t, int32_t, void *);
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t);

static uint32_t node_id;
static char node_ip_address[INET_ADDRSTRLEN];

TaskHandle_t createMqttTask(char *ip_address)
{
  uint64_t chipmacid;
  ESP_ERROR_CHECK(esp_efuse_mac_get_default((uint8_t *)(&chipmacid)));
  node_id = (uint32_t)(chipmacid >> 16);
  memcpy(node_ip_address, ip_address, INET_ADDRSTRLEN);

  xTaskHandle xHandle = NULL;

  xTaskCreate(mqttTask, MQTT_TASK_TAG, 4096, NULL, 5, &xHandle);
  if (xHandle == NULL)
  {
    ESP_LOGE(MQTT_TASK_TAG, "Could not create task");
    abort();
  }
  return xHandle;
}

static void mqttTask(void *_)
{
  char name[14];
  char topic_state[30];

  snprintf(name, 14, "node-%08X", node_id);
  snprintf(topic_state, 30, "/nodes/%08X/state", node_id);

  ESP_LOGI(MQTT_TASK_TAG, "Registering as %s", name);

  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = "mqtt://192.168.11.113",
      .port = 1883,
      .lwt_topic = topic_state,
      .lwt_qos = 1,
      .lwt_msg = "0",
      .lwt_retain = true};
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client));

  ESP_ERROR_CHECK(esp_mqtt_client_start(client));

  ESP_LOGI(MQTT_TASK_TAG, "Mqtt connection started");
  vTaskDelete(NULL);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGI(MQTT_TASK_TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  mqtt_event_handler_cb(event_data);
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  switch (event->event_id)
  {
  case MQTT_EVENT_CONNECTED:
  {
    char topic_state[30];
    char topic_settings[35];

    snprintf(topic_state, 30, "/nodes/%08X/state", node_id);
    snprintf(topic_settings, 35, "/nodes/%08X/settings/#", node_id);

    ESP_LOGI(MQTT_TASK_TAG, "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_publish(client, topic_state, node_ip_address, 0, 1, 1);
    ESP_LOGI(MQTT_TASK_TAG, "sent publish successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(client, topic_settings, 1);
    ESP_LOGI(MQTT_TASK_TAG, "sent subscribe successful, msg_id=%d", msg_id);
    break;
  }
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(MQTT_TASK_TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(MQTT_TASK_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(MQTT_TASK_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(MQTT_TASK_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(MQTT_TASK_TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(MQTT_TASK_TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(MQTT_TASK_TAG, "Other event id:%d", event->event_id);
    break;
  }
  return ESP_OK;
}
