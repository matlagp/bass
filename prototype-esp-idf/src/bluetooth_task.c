#include "bluetooth_task.h"

#define SPP_TAG "SPP_ACCEPTOR_DEMO"
#define SPP_SERVER_NAME "SPP_SERVER"
#define EXAMPLE_DEVICE_NAME "ESP_SPP_ACCEPTOR"
#define SPP_SHOW_DATA 0
#define SPP_SHOW_SPEED 1
#define SPP_SHOW_MODE SPP_SHOW_DATA /*Choose show mode: show data or speed*/

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  switch (event)
  {
  case ESP_SPP_INIT_EVT:
  {
    uint64_t chipmacid;
    ESP_ERROR_CHECK(esp_efuse_mac_get_default((uint8_t *)(&chipmacid)));
    uint32_t node_id = (uint32_t)(chipmacid >> 16);
    char name[14];
    snprintf(name, 14, "node-%08X", node_id);

    ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
    esp_bt_dev_set_device_name(name);
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    ESP_ERROR_CHECK(esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "spp"));
    break;
  }
  case ESP_SPP_CLOSE_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT");
    break;
  case ESP_SPP_DATA_IND_EVT:
    ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d data=%s",
             param->data_ind.len, param->data_ind.handle, param->data_ind.data);
    esp_log_buffer_hex("", param->data_ind.data, param->data_ind.len);
    break;
  default:
    break;
  }
}
void createBluetoothTask()
{
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));

  ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

  ESP_ERROR_CHECK(esp_bluedroid_init());

  ESP_ERROR_CHECK(esp_bluedroid_enable());

  ESP_ERROR_CHECK(esp_spp_register_callback(esp_spp_cb));

  ESP_ERROR_CHECK(esp_spp_init(esp_spp_mode));

  /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
  esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
  esp_bt_pin_code_t pin_code;
  esp_bt_gap_set_pin(pin_type, 0, pin_code);
}
