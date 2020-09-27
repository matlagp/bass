#include <stdio.h>
#include <string.h>
#include <driver/i2c.h>
#include <driver/i2s.h>
#include "esp_err.h"
#include "esp_log.h"

#ifndef WM8960_H
#define WM8960_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22

#define I2C_BUS_NO I2C_NUM_0

// This is what Wikipedia says (https://en.wikipedia.org/wiki/I%C2%B2C)
#define ACK_VAL 0
#define NACK_VAL 1

#define ACK_CHECK_EN 1
#define ACK_CHECK_DIS 1

#define CODEC_ADDR 0x1A

  // static void wm8960_i2c_init();
  // static esp_err_t i2c_trans(i2c_port_t i2c_num, uint8_t addr, void *txdata, uint8_t txlen);
  // static esp_err_t i2c_recv(i2c_port_t i2c_num, uint8_t addr, void *rxdata, uint8_t rxlen);
  // static esp_err_t write_register_i2c(uint8_t slave_id, uint32_t reg_addr, uint32_t reg_val);
  // static esp_err_t read_register_i2c(uint8_t slave_id, uint8_t reg_addr);

  esp_err_t wm8960_init();
  esp_err_t wm8960_set_vol(int vol);
  esp_err_t wm8960_set_mute(bool mute);
  esp_err_t wm8960_get_volume(uint8_t *vol);

#ifdef __cplusplus
}
#endif

#endif
