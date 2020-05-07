#include <Arduino.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"

static int32_t bt_app_a2d_data_cb(uint8_t *data, int32_t len)
{
  // Serial.println(len);

    if (len < 0 || data == NULL) {
        return 0;
    }

    // generate random sequence
    int val = rand() % (1 << 16);
    for (int i = 0; i < (len >> 1); i++) {
        data[(i << 1)] = val & 0xff;
        data[(i << 1) + 1] = (val >> 8) & 0xff;
    }

    return len;
}

static void esp_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
  Serial.printf("callback: %d\n", event);
  if (event == 0) {
    Serial.printf("%d\n", param->conn_stat.state);
    if (param->conn_stat.state == 2) {
      esp_a2d_media_ctrl(ESP_A2D_MEDIA_CTRL_START);
    }
  }
}

static void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch(event){
        case ESP_BT_GAP_DISC_RES_EVT:
            log_i("ESP_BT_GAP_DISC_RES_EVT");
            break;
        case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
            log_i("ESP_BT_GAP_DISC_STATE_CHANGED_EVT");
            break;
        case ESP_BT_GAP_RMT_SRVCS_EVT:
            log_i( "ESP_BT_GAP_RMT_SRVCS_EVT");
            break;
        case ESP_BT_GAP_RMT_SRVC_REC_EVT:
            log_i("ESP_BT_GAP_RMT_SRVC_REC_EVT");
            break;
        case ESP_BT_GAP_AUTH_CMPL_EVT:
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                log_v("authentication success: %s", param->auth_cmpl.device_name);
            } else {
                log_e("authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        case ESP_BT_GAP_PIN_REQ_EVT:
            // default pairing pins
            log_i("ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
            if (param->pin_req.min_16_digit) {
                log_i("Input pin code: 0000 0000 0000 0000");
                esp_bt_pin_code_t pin_code;
                memset(pin_code, '0', ESP_BT_PIN_CODE_LEN);
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
            } else {
                log_i("Input pin code: 1234");
                esp_bt_pin_code_t pin_code;
                memcpy(pin_code, "1234", 4);
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            }
            break;
        case ESP_BT_GAP_CFM_REQ_EVT:
            log_i("ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
            esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
            break;
        case ESP_BT_GAP_KEY_NOTIF_EVT:
            log_i("ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
            break;
        case ESP_BT_GAP_KEY_REQ_EVT:
            log_i("ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
            break;
        default:
            break;
    }
}

void setup() {
  Serial.begin(9600);

  // ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

  
    if (!btStarted() && !btStart()){
        log_e("initialize controller failed");
        return;
    }
    
    esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
    if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED){
        if (esp_bluedroid_init()) {
            log_e("initialize bluedroid failed");
            return;
        }
    }
    
    if (bt_state != ESP_BLUEDROID_STATUS_ENABLED){
        if (esp_bluedroid_enable()) {
            log_e("enable bluedroid failed");
            return;
        }
    }

  // esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

  // btStart();

  // if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
  //   ESP_LOGE(BT_AV_TAG, "%s controller init failed\n", __func__);
  //   return;
  // }

  // if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) != ESP_OK) {
  //   ESP_LOGE(BT_AV_TAG, "%s enable controller failed\n", __func__);
  //   return;
  // }

  // if (esp_bluedroid_init() != ESP_OK) {
  //   ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed\n", __func__);
  //   return;
  // }

  // if (esp_bluedroid_enable() != ESP_OK) {
  //   ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed\n", __func__);
  //   return;
  // };

  esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
  esp_bt_pin_code_t pin_code;
  if (esp_bt_gap_set_pin(pin_type, 0, pin_code) != ESP_OK) {
    ESP_LOGE(BT_AV_TAG, "%s gap set pin failed\n", __func__);
    return;
  }

  if (esp_bt_gap_register_callback(esp_bt_gap_cb) != ESP_OK) {
    ESP_LOGE(BT_AV_TAG, "%s register gap callback failed\n", __func__);
    return;
  }

  if (esp_a2d_register_callback(esp_a2d_cb) != ESP_OK) {
    ESP_LOGE(BT_AV_TAG, "%s register callback failed\n", __func__);
    return;
  }

  if (esp_a2d_source_init() != ESP_OK) {
    ESP_LOGE(BT_AV_TAG, "%s source init failed\n", __func__);
    return;
  }

  if (esp_a2d_source_register_data_callback(bt_app_a2d_data_cb)) {
    ESP_LOGE(BT_AV_TAG, "%s register data callback failed\n", __func__);
    return;
  }

  esp_bd_addr_t remote_bda = {0xE8, 0xEC, 0xA3, 0x05, 0x27, 0x4D};
  if (esp_a2d_source_connect(remote_bda) != ESP_OK) {
    ESP_LOGE(BT_AV_TAG, "%s connect failed\n", __func__);
    return;
  }

  Serial.println("Success!");

  // esp_a2d_source_register_data_callback(bt_app_a2d_data_cb);
}

void loop() {
}