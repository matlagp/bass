#include <Arduino.h>

#include "esp_a2dp_api.h"
#include "esp_bt.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"

static int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len)
{
  if (len < 0 || data == NULL) return 0;

  // generate random sequence
  int val = rand() % (1 << 16);
  for (int i = 0; i < (len >> 1); i++)
  {
    data[(i << 1)] = val & 0xff;
    data[(i << 1) + 1] = (val >> 8) & 0xff;
  }

  return len;
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

void setup()
{
  Serial.begin(9600);

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

  Serial.println("Success!");
}

void loop()
{
}