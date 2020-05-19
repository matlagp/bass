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

const int buff_len = (2 << 14) + 1;

typedef struct {
  uint8_t *start;       // ptr to first buffer slot
  uint8_t *end;         // ptr to last buffer slot
  uint8_t *valid_start; // ptr to first filled spot (buffer is empty if valid_start == valid_end)
  uint8_t *valid_end;   // ptr to first unfilled slot
  SemaphoreHandle_t mutex;
  uint8_t *buffer;
} buffer;

buffer buff;

void init_buffer(buffer *buffer) {
  buffer->buffer = (uint8_t*) calloc(buff_len, 1);
  assert(buffer != nullptr);
  buffer->start = &buffer->buffer[0];
  buffer->end = &buffer->buffer[buff_len-1];
  buffer->valid_start = buffer->start;
  buffer->valid_end = buffer->start;
  buffer->mutex = xSemaphoreCreateMutex();
}

inline uint8_t *buffer_ptr_inc(buffer *buffer, uint8_t *ptr) {
  return ptr + 1 <= buffer->end ? ptr + 1 : buffer->start;
}

int32_t buffer_taken(buffer *buffer) {
  int32_t ret = 0;

  if (buffer->valid_end > buffer->valid_start) {
    ret += buffer->valid_end - buffer->valid_start;
  } else if (buffer->valid_end < buffer->valid_start) {
    ret += buffer->end - buffer->valid_start + 1;
    ret += buffer->valid_end - buffer->start;
  }

  return ret;
}

int32_t buffer_free_cont(buffer *buffer) {
  int32_t ret = 0;
  //xSemaphoreTake(buffer->mutex, portMAX_DELAY);

  if (buffer->valid_start <= buffer->valid_end) {
    ret = buffer->end - buffer->valid_end;
    if (buffer->valid_start != buffer->start) ret += 1;
  } else if (buffer->valid_start > buffer->valid_end) {
    ret = buffer->valid_start - buffer->valid_end - 1;
  }

  //xSemaphoreGive(buffer->mutex);
  return ret;
}

bool buffer_write(buffer *buffer, uint8_t data) {
  //xSemaphoreTake(buffer->mutex, portMAX_DELAY);

  uint8_t *next = buffer_ptr_inc(buffer, buffer->valid_end);
  if (next == buffer->valid_start) 
  {
    //xSemaphoreGive(buffer->mutex);
    return false;
  }

  *buffer->valid_end = data;
  buffer->valid_end = next;

  //xSemaphoreGive(buffer->mutex);
  return true;
}

uint32_t buffer_read(buffer *buffer, uint8_t *data, int32_t len) {
  //xSemaphoreTake(buffer->mutex, portMAX_DELAY);

  int32_t available = buffer_taken(buffer);

  if (buffer->valid_end == buffer->valid_start) {
    //xSemaphoreGive(buffer->mutex);
    return 0;
  }

  if (buffer->valid_end > buffer->valid_start) {
    int32_t l = buffer->valid_end - buffer->valid_start;
    l = len < l ? len : l;
    memcpy(data, buffer->valid_start, l);
    if (available > 128) buffer->valid_start += l;

    //xSemaphoreGive(buffer->mutex);
    return l;
  } else {
    int32_t l1 = buffer->end - buffer->valid_start + 1;
    int32_t l2 = 0;
    l1 = len < l1 ? len : l1;
    memcpy(data, buffer->valid_start, l1);
    buffer->valid_start += l1;
    if (available > 128 && buffer->valid_start > buffer->end) buffer->valid_start = buffer->start;

    if (l1 < len) {
      l2 = buffer->valid_end - buffer->start;
      l2 = (len-l1) < l2 ? (len-l1) : l2;

      memcpy(&data[l1], buffer->start, l2);
      if (available > 128) buffer->valid_start += l2;
    }

    //xSemaphoreGive(buffer->mutex);
    return l1 + l2;
  }
}

bool buffer_read(buffer *buffer, uint8_t *data) {
  if (buffer->valid_end == buffer->valid_start) return false;

  *data = *buffer->valid_start;
  buffer->valid_start = buffer_ptr_inc(buffer, buffer->valid_start);
  return true;
}

static int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len)
{
  memcpy(data, buff.buffer, len);
  return len;
  // return buffer_read(&buff, data, len);
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

  uint8_t devnull[1024];

  if (client.connected()) {
    if (client.available()) {
        // client.readBytes(devnull, 1024);
      int32_t len = buffer_free_cont(&buff);
      if (len == 0) {
        client.readBytes(devnull, 256);
        return;
      }
      len = len > 256 ? 256 : len;
      len = client.readBytes(buff.valid_end, len);
      if (buff.valid_end + len > buff.end) {
        buff.valid_end = buff.start;
      } else {
        buff.valid_end += len;
      }
    }
  } else {
    client = tcp.available();
    if (client.available()) {
      Serial.println("New client connected");
    }
  }
}