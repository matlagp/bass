#include "i2s_setup.h"

void init_i2s()
{
  i2s_config_t i2s_config = {
      .mode = I2S_MODE_MASTER | I2S_MODE_TX, // Only TX
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = 16,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, //2-channels
      .communication_format = I2S_COMM_FORMAT_STAND_MSB,
      .dma_buf_count = DMA_BUFFER_COUNT,
      .dma_buf_len = DMA_BUFFER_LENGTH, // in SAMPLES
      .use_apll = false,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 //Interrupt level 1
  };
  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCK_IO,
      .ws_io_num = I2S_WS_IO,
      .data_out_num = I2S_DO_IO,
      .data_in_num = I2S_DI_IO //Not used
  };

  ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL));
  ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM, &pin_config));

  //enable MCLK on GPIO0
  REG_WRITE(PIN_CTRL, 0xFF0);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
}
