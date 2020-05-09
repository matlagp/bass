#include <Arduino.h>
#include <WiFi.h>

#include "esp_a2dp_api.h"
#include "esp_bt.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"

const char *ssid = "...";
const char *pwd = "...";

WiFiServer tcp;
WiFiClient client;
bool connected = false;

const int buff_len = 2 << 14;

typedef struct {
  uint8_t *start;
  uint8_t *end;
  uint8_t *valid_start;
  uint8_t *valid_end;
  uint8_t buffer[buff_len];
} buffer;

buffer buff;

void init_buffer(buffer *buffer) {
  buffer->start = &buffer->buffer[0];
  buffer->end = &buffer->buffer[buff_len-1];
  buffer->valid_start = buffer->start;
  buffer->valid_end = buffer->start;
}

inline uint8_t *buffer_ptr_inc(buffer *buffer, uint8_t *ptr) {
  return ptr + 1 <= buffer->end ? ptr + 1 : buffer->start;
}

bool buffer_write(buffer *buffer, uint8_t data) {
  uint8_t *next = buffer_ptr_inc(buffer, buffer->valid_end);
  if (next == buffer->valid_start) return false;

  buffer->valid_end = next;
  *buffer->valid_end = data;
  return true;
}

bool buffer_read(buffer *buffer, uint8_t *data) {
  if (buffer->valid_end == buffer->valid_start) return false;

  *data = *buffer->valid_start;
  buffer->valid_start = buffer_ptr_inc(buffer, buffer->valid_start);
  return true;
}

static int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len)
{
  int i;

  for (i = 0; i < len; i++) {
    if (!buffer_read(&buff, &data[i])) break;
  }

  return i;
}

static void esp_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
  Serial.printf("callback: %d\n", event);
  if (event == 0)
  {
    Serial.printf("%d\n", param->conn_stat.state);
    if (param->conn_stat.state == 2)
    {
      esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_START);
    }
  }
}

static void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  

          tcp.begin(2137);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}

void connectToWiFi(const char * ssid, const char * pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, pwd);
  Serial.println("Waiting for WIFI connection...");
}

void setup()
{
  Serial.begin(9600);

  init_buffer(&buff);

  if (!btStarted() && !btStart())
  {
    log_e("initialize controller failed");
    return;
  }

  esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
  if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED)
  {
    if (esp_bluedroid_init())
    {
      log_e("initialize bluedroid failed");
      return;
    }
  }

  if (bt_state != ESP_BLUEDROID_STATUS_ENABLED)
  {
    if (esp_bluedroid_enable())
    {
      log_e("enable bluedroid failed");
      return;
    }
  }

  if (esp_a2d_register_callback(esp_a2d_cb) != ESP_OK)
  {
    ESP_LOGE(BT_AV_TAG, "%s register callback failed\n", __func__);
    return;
  }

  if (esp_a2d_source_init() != ESP_OK)
  {
    ESP_LOGE(BT_AV_TAG, "%s source init failed\n", __func__);
    return;
  }

  if (esp_a2d_source_register_data_callback(bt_app_a2d_data_cb))
  {
    ESP_LOGE(BT_AV_TAG, "%s register data callback failed\n", __func__);
    return;
  }

  esp_bd_addr_t remote_bda = {0xE8, 0xEC, 0xA3, 0x05, 0x27, 0x4D};
  if (esp_a2d_source_connect(remote_bda) != ESP_OK)
  {
    ESP_LOGE(BT_AV_TAG, "%s connect failed\n", __func__);
    return;
  }

  connectToWiFi(ssid, pwd);

  Serial.println("Success!");
}

void loop()
{
  if (!connected) return;

  uint8_t data;

  if (client.connected()) {
    if (client.available()) {
      if (client.read(&data, 1) == 1) {
        buffer_write(&buff, data);
      }
    }
  } else {
    client = tcp.available();
    if (client.available()) {
      Serial.println("New client connected");
    }
  }
}