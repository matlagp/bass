#include "sine.h"

void setup_triangle_sine_waves(int bits)
{
  int *samples_data = (int *)malloc(((bits + 8) / 16) * SAMPLE_PER_CYCLE * 4);
  unsigned int i, sample_val;
  double sin_float, triangle_float, triangle_step = (double)pow(2, bits) / SAMPLE_PER_CYCLE;
  size_t i2s_bytes_write = 0;

  printf("\r\nTest bits=%d free mem=%d, written data=%d\n", bits, esp_get_free_heap_size(), ((bits + 8) / 16) * SAMPLE_PER_CYCLE * 4);

  triangle_float = -(pow(2, bits) / 2 - 1);

  for (i = 0; i < SAMPLE_PER_CYCLE; i++)
  {
    sin_float = sin(i * 2 * PI / SAMPLE_PER_CYCLE);
    if (sin_float >= 0)
      triangle_float += triangle_step;
    else
      triangle_float -= triangle_step;

    sin_float *= (pow(2, bits) / 2 - 1);

    if (bits == 16)
    {
      sample_val = 0;
      sample_val += (short)triangle_float;
      sample_val = sample_val << 16;
      sample_val += (short)sin_float;
      samples_data[i] = sample_val;
    }
    else if (bits == 24)
    { //1-bytes unused
      samples_data[i * 2] = ((int)triangle_float) << 8;
      samples_data[i * 2 + 1] = ((int)sin_float) << 8;
    }
    else
    {
      samples_data[i * 2] = ((int)triangle_float);
      samples_data[i * 2 + 1] = ((int)sin_float);
    }
  }

  i2s_set_clk(I2S_NUM, SAMPLE_RATE, bits, I2S_CHANNEL_STEREO);
  //Using push
  // for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
  //     if (bits == 16)
  //         i2s_push_sample(0, &samples_data[i], 100);
  //     else
  //         i2s_push_sample(0, &samples_data[i*2], 100);
  // }
  // or write
  i2s_write(I2S_NUM, samples_data, ((bits + 8) / 16) * SAMPLE_PER_CYCLE * 4, &i2s_bytes_write, 100);

  free(samples_data);
}

void init_i2s()
{
  i2s_config_t i2s_config = {
      .mode = I2S_MODE_MASTER | I2S_MODE_TX, // Only TX
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = 16,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, //2-channels
      .communication_format = I2S_COMM_FORMAT_STAND_MSB,
      .dma_buf_count = 6,
      .dma_buf_len = 60,
      .use_apll = false,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 //Interrupt level 1
  };
  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCK_IO,
      .ws_io_num = I2S_WS_IO,
      .data_out_num = I2S_DO_IO,
      .data_in_num = I2S_DI_IO //Not used
  };
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
}