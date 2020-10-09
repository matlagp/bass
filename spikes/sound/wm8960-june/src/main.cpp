#include <Arduino.h>
#include <Wire.h>
#include "WM8960.h"
#include "casio.h"

#include "driver/i2s.h"
#include "freertos/queue.h"

#define I2C_SDA 14
#define I2C_SCL 27

static const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

static const i2s_config_t i2s_config = {
  .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
  .sample_rate = 44100,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, /* the DAC module will only take the 8bits from MSB */
  .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
  .communication_format = I2S_COMM_FORMAT_I2S_MSB,
  .intr_alloc_flags = 0, // default interrupt priority
  .dma_buf_count = 8,
  .dma_buf_len = 64,
  .use_apll = false
};

static const i2s_pin_config_t pin_config = {
  .bck_io_num = I2S_CLK,
  .ws_io_num = I2S_WS,
  .data_out_num = I2S_TXSDA,
  .data_in_num = I2S_PIN_NO_CHANGE
};

unsigned int bytes_read = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SDA, I2C_SCL);

  byte wm_init_result = WM8960.begin();
  if (wm_init_result == 0) {
    Serial.printf("DAC initialized successfully\n");
  } else {
    Serial.printf("DAC initialization failed: %d\n", wm_init_result);
  }

  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin(i2s_num, &pin_config);
  i2s_set_sample_rates(i2s_num, 44100);
}

void loop() {
  if (bytes_read < casio_len) {
    esp_err_t err = i2s_write(i2s_num, casio + bytes_read, casio_len - bytes_read, &bytes_read, 1000);
    Serial.printf("Wrote %d bytes so far\n", bytes_read);
    if (err) {
      Serial.printf("Error while pushing data: %d\n", err);
    }
  } else {
    Serial.println("Playback finished");
    bytes_read = 0;
    delay(1000);
  }
}