#include "ir_task.h"

static void irTask(void *);
static void processIrCmd(uint32_t, bool);
static int mqtt_publish(const char *, const char *);

static TickType_t lastCmdTickCount = 0;

static char topic_volume[27];
static char topic_bass[25];
static char topic_mid[24];
static char topic_treble[27];

typedef enum ir_code_t {
    VOL_DOWN = 0xf807,
    VOL_UP = 0xea15,
    BASS_DOWN = 0xf30c,
    BASS_UP = 0xe718,
    MID_DOWN = 0xf708,
    MID_UP = 0xe31c,
    TREBLE_DOWN = 0xbd42,
    TREBLE_UP = 0xad52
} ir_code_t;

TaskHandle_t createIrTask() {
  xTaskHandle xHandle = NULL;

  xTaskCreate(irTask, IR_TASK_TAG, 4096, NULL, 5, &xHandle);
  if (xHandle == NULL) {
    ESP_LOGE(IR_TASK_TAG, "Could not create task");
    abort();
  }

  return xHandle;
}

static void irTask(void *_) {
  uint32_t addr = 0;
  uint32_t cmd = 0;
  bool repeat = false;
  uint32_t length = 0;
  rmt_item32_t *ir_data = NULL;
  RingbufHandle_t buffer = NULL;

  snprintf(topic_volume, 27, "/nodes/%08X/set/volume", node_id);
  snprintf(topic_bass, 25, "/nodes/%08X/set/bass", node_id);
  snprintf(topic_mid, 24, "/nodes/%08X/set/mid", node_id);
  snprintf(topic_treble, 27, "/nodes/%08X/set/treble", node_id);

  ir_parser_config_t ir_parser_config = IR_PARSER_DEFAULT_CONFIG((ir_dev_t)IR_CHANNEL_NUM);
  ir_parser_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT;
  ir_parser_t *ir_parser = ir_parser_rmt_new_nec(&ir_parser_config);

  rmt_get_ringbuf_handle(IR_CHANNEL_NUM, &buffer);
  rmt_rx_start(IR_CHANNEL_NUM, true);
  for (;;) {
    ir_data = (rmt_item32_t *)xRingbufferReceive(buffer, &length, portMAX_DELAY);
    if (ir_data) {
      length /= 4;
      if (ir_parser->input(ir_parser, ir_data, length) == ESP_OK &&
          ir_parser->get_scan_code(ir_parser, &addr, &cmd, &repeat) == ESP_OK) {
        ESP_LOGI(IR_TASK_TAG, "Scan Code %s --- addr: 0x%04x cmd: 0x%04x",
                 repeat ? "(repeat)" : "", addr, cmd);
        processIrCmd(cmd, repeat);
      } else {
        ESP_LOGD(IR_TASK_TAG, "Got IR packet that couldn't be decoded");
      }
      vRingbufferReturnItem(buffer, (void *)ir_data);
    }
  }
}

static void processIrCmd(uint32_t cmd, bool repeat) {
  TickType_t tickCount = xTaskGetTickCount();
  if (repeat && (tickCount - lastCmdTickCount < pdMS_TO_TICKS(200))) {
    return;
  }
  lastCmdTickCount = tickCount;

  switch (cmd) {
    case VOL_DOWN:
      mqtt_publish(topic_volume, "-5");
      break;
    case VOL_UP:
      mqtt_publish(topic_volume, "+5");
      break;
    case BASS_DOWN:
      mqtt_publish(topic_bass, "-0.5");
      break;
    case BASS_UP:
      mqtt_publish(topic_bass, "+0.5");
      break;
    case MID_DOWN:
      mqtt_publish(topic_mid, "-0.5");
      break;
    case MID_UP:
      mqtt_publish(topic_mid, "+0.5");
      break;
    case TREBLE_DOWN:
      mqtt_publish(topic_treble, "-0.5");
      break;
    case TREBLE_UP:
      mqtt_publish(topic_treble, "+0.5");
      break;
    default:
      ESP_LOGI(IR_TASK_TAG, "Unsupported command: %04x", cmd);
      break;
  }
}

static int mqtt_publish(const char *topic, const char *message) {
  return esp_mqtt_client_publish(mqtt_client, topic, message, 0, 1, 0);
}
