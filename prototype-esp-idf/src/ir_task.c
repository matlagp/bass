#include "ir_task.h"

static void irTask(void *);

TaskHandle_t createIrTask() {
  xTaskHandle xHandle = NULL;

  xTaskCreate(irTask, IR_TASK_TAG, 2048, NULL, 5, &xHandle);
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
      } else {
        ESP_LOGE(IR_TASK_TAG, "Problem while decoding IR data");
      }
      vRingbufferReturnItem(buffer, (void *)ir_data);
    }
  }
}
