#include "bluetooth_task.h"

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
static void parse_credentials(esp_spp_cb_param_t *param);
static void free_buffers(void);

static bool received_credentials = false;
static char *wifi_ssid = NULL;
static char *wifi_password = NULL;
static char server_ip_address[INET_ADDRSTRLEN];

static void (*on_credentials_received_handler)(char *ssid, char *password, char *ip);

void createBluetoothTask(void (*on_credentials_received)(char *ssid, char *password, char *ip))
{
  on_credentials_received_handler = on_credentials_received;

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

void cleanupBluetooth(bool bluetoothInitialized)
{
  free_buffers();

  if (bluetoothInitialized) {
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit());
    ESP_ERROR_CHECK(esp_bt_controller_disable());
    ESP_ERROR_CHECK(esp_bt_controller_deinit());
  }
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
}

void retryBluetooth(void)
{
  received_credentials = false;
  free_buffers();

  ESP_ERROR_CHECK(esp_spp_register_callback(esp_spp_cb));
  ESP_ERROR_CHECK(esp_spp_init(esp_spp_mode));
}

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

    ESP_LOGI(BLUETOOTH_TASK_TAG, "ESP_SPP_INIT_EVT");
    ESP_ERROR_CHECK(esp_bt_dev_set_device_name(name));
    ESP_ERROR_CHECK(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));
    ESP_ERROR_CHECK(esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "spp"));
    break;
  }
  case ESP_SPP_CLOSE_EVT:
    ESP_LOGI(BLUETOOTH_TASK_TAG, "ESP_SPP_CLOSE_EVT");
    ESP_ERROR_CHECK(esp_spp_deinit());
    if (!received_credentials)
    {
      ESP_LOGI(BLUETOOTH_TASK_TAG, "Restarting bluetooth server");
      ESP_ERROR_CHECK(esp_spp_register_callback(esp_spp_cb));
      ESP_ERROR_CHECK(esp_spp_init(esp_spp_mode));
    }
    else
    {
      on_credentials_received_handler(wifi_ssid, wifi_password, server_ip_address);
    }
    break;
  case ESP_SPP_DATA_IND_EVT:
    ESP_LOGI(BLUETOOTH_TASK_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d data=%s",
             param->data_ind.len, param->data_ind.handle, param->data_ind.data);
    esp_log_buffer_hex("", param->data_ind.data, param->data_ind.len);

    parse_credentials(param);

    esp_spp_disconnect(param->data_ind.handle);
    break;
  default:
    break;
  }
}

static void parse_credentials(esp_spp_cb_param_t *param)
{
  received_credentials = false;
  free_buffers();

  enum parse_state_t
  {
    IS_1 = 0,
    SEEKING_SSID_END = 1,
    IS_2 = 2,
    SEEKING_PASSWORD_END = 3,
    IS_3 = 4,
    SEEKING_IP_END = 5,
    DONE = 6
  };

  enum parse_state_t state = IS_1;

  char *word_start = NULL;

  for (int i = 0; i < param->data_ind.len; i++)
  {
    char current_char = param->data_ind.data[i];

    switch (state)
    {
    case IS_1:
      if (current_char != '1')
      {
        ESP_LOGW(BLUETOOTH_TASK_TAG, "Expected '1' got '%c'", current_char);
        return;
      }
      word_start = (char *)&param->data_ind.data[i + 1];
      state += 1;
      break;

    case IS_2:
      if (current_char != '2')
      {
        ESP_LOGW(BLUETOOTH_TASK_TAG, "Expected '2' got '%c'", current_char);
        return;
      }
      word_start = (char *)&param->data_ind.data[i + 1];
      state += 1;
      break;

    case IS_3:
      if (current_char != '3')
      {
        ESP_LOGW(BLUETOOTH_TASK_TAG, "Expected '3' got '%c'", current_char);
        return;
      }
      word_start = (char *)&param->data_ind.data[i + 1];
      state += 1;
      break;

    case SEEKING_SSID_END:
      if (current_char != '\n')
        break;
      param->data_ind.data[i] = '\0';

      wifi_ssid = malloc(strlen(word_start) + 1);
      if (wifi_ssid == NULL)
      {
        ESP_LOGW(BLUETOOTH_TASK_TAG, "Could not allocate wifi_ssid buffer");
        return;
      }

      memcpy(wifi_ssid, word_start, strlen(word_start) + 1);

      state += 1;
      break;

    case SEEKING_PASSWORD_END:
      if (current_char != '\n')
        break;
      param->data_ind.data[i] = '\0';

      wifi_password = malloc(strlen(word_start) + 1);
      if (wifi_password == NULL)
      {
        ESP_LOGW(BLUETOOTH_TASK_TAG, "Could not allocate wifi_password buffer");
        return;
      }

      memcpy(wifi_password, word_start, strlen(word_start) + 1);

      state += 1;
      break;

    case SEEKING_IP_END:
      if (current_char != '\n')
        break;
      param->data_ind.data[i] = '\0';

      if (strlen(word_start) + 1 > INET_ADDRSTRLEN)
      {
        ESP_LOGW(BLUETOOTH_TASK_TAG, "IP address is too long");
        return;
      }

      memcpy(server_ip_address, word_start, strlen(word_start) + 1);

      state += 1;
      break;

    case DONE:
      ESP_LOGW(BLUETOOTH_TASK_TAG, "Extra characters after valid config sequence");
      return;
    }
  }

  if (state == DONE)
  {
    received_credentials = true;
    ESP_LOGI(BLUETOOTH_TASK_TAG, "SSID: (%u) %s", strlen(wifi_ssid), wifi_ssid);
    ESP_LOGI(BLUETOOTH_TASK_TAG, "PASSWORD: (%u) %s", strlen(wifi_password), wifi_password);
    ESP_LOGI(BLUETOOTH_TASK_TAG, "IP: (%u) %s", strlen(server_ip_address), server_ip_address);
  }
  else
  {
    ESP_LOGW(BLUETOOTH_TASK_TAG, "Finished in an unexpected state: %d", state);
  }
}

static void free_buffers(void)
{
  if (wifi_ssid != NULL)
  {
    free(wifi_ssid);
    wifi_ssid = NULL;
  }
  if (wifi_password != NULL)
  {
    free(wifi_password);
    wifi_password = NULL;
  }
}
