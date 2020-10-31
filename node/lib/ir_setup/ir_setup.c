#include "ir_setup.h"

void init_ir() {
  rmt_config_t rmt_rx_config = RMT_DEFAULT_CONFIG_RX(IR_DI_IO, IR_CHANNEL_NUM);
  ESP_ERROR_CHECK(rmt_config(&rmt_rx_config));
  ESP_ERROR_CHECK(rmt_driver_install(IR_CHANNEL_NUM, 1024, 0));
}
