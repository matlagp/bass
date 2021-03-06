#include <stdio.h>
#include <string.h>
#include <driver/i2c.h>
#include <driver/i2s.h>
#include "esp_err.h"
#include "esp_log.h"

#include "wm8960.h"
static const char *TAG = "WM8960";

static uint8_t volume;

static void wm8960_i2c_init()
{
  esp_err_t rc;
  i2c_config_t i2c_conf;
  memset(&i2c_conf, 0x00, sizeof(i2c_conf));
  i2c_conf.mode = I2C_MODE_MASTER;
  i2c_conf.sda_io_num = SDA_PIN;
  i2c_conf.scl_io_num = SCL_PIN;
  i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  i2c_conf.master.clk_speed = 526 * 100;
  // ESP_LOGI(TAG, "Configuring I2C");
  rc = i2c_param_config(I2C_BUS_NO, &i2c_conf);
  // ESP_LOGD(TAG, "I2C Param Config: %s", esp_err_to_name(rc));
  rc = i2c_driver_install(I2C_BUS_NO, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_LEVEL1);
  // ESP_LOGD(TAG, "I2C Driver Install; %s", esp_err_to_name(rc));
}

static esp_err_t i2c_trans(i2c_port_t i2c_num, uint8_t addr, void *txdata, uint8_t txlen)
{
  esp_err_t rc;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  // ESP_LOGE(TAG, "CMD Handle: %s", esp_err_to_name(rc));
  rc = i2c_master_start(cmd);
  // ESP_LOGE(TAG, "[W] Master_start: %s", esp_err_to_name(rc));
  rc = i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  // ESP_LOGE(TAG, "[W] Master_write_byte: %s", esp_err_to_name(rc));
  rc = i2c_master_write(cmd, txdata, txlen, ACK_CHECK_EN);
  // ESP_LOGE(TAG, "[W] Master_write: %s", esp_err_to_name(rc));
  rc = i2c_master_stop(cmd);
  // ESP_LOGE(TAG, "[W] Master stop: %s", esp_err_to_name(rc));
  rc = i2c_master_cmd_begin(I2C_BUS_NO, cmd, 10);
  // ESP_LOGE(TAG, "[W] CMD begin: %s", esp_err_to_name(rc));
  i2c_cmd_link_delete(cmd);
  return rc;
}

static esp_err_t i2c_recv(i2c_port_t i2c_num, uint8_t addr, void *rxdata, uint8_t rxlen)
{
  esp_err_t rc;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (CODEC_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_DIS);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1), ACK_CHECK_EN);
  if (rxlen > 1)
  {
    i2c_master_read(cmd, rxdata, rxlen - 1, ACK_VAL);
  }
  i2c_master_read_byte(cmd, rxdata + rxlen - 1, NACK_VAL);
  i2c_master_stop(cmd);
  rc = i2c_master_cmd_begin(i2c_num, cmd, 10);
  printf("Recv: %d\n", rc);
  i2c_cmd_link_delete(cmd);
  return rc;
}

static esp_err_t write_register_i2c(uint8_t slave_id, uint32_t reg_addr, uint32_t reg_val)
{
  esp_err_t rc;
  // int retries = 0;
  // do
  // {
  uint8_t buff[3];
  buff[0] = (reg_addr << 1) | ((reg_val >> 8) & 0x0f);
  buff[1] = reg_val & 0xff;
  // printf("0x%x: 0x%x \t %x %x\n", reg_addr, reg_val, buff[0], buff[1]);
  rc = i2c_trans(I2C_BUS_NO, slave_id, buff, 2);
  //   retries++;
  // } while (rc != ESP_OK && retries < 50);
  return rc;
}

const uint16_t wm8960_reg_defaults[] = {
    0x0, // Left Input volume
    0x00a7,
    0x1, // Right Input volume
    0x00a7,
    0x2, // LOUT1 volume
    0x0000,
    0x3, // LOUT2 volume
    0x0000,
    0x4, // Clocking (1)
    0x0000,
    0x5,    // ADC & DAC Control (CTR1)
    0x0000, // soft mute disabled (important)
    0x6,    // ADC & DAC Control (CTR2)
    0x0000,
    0x7, // Audio interface (16 bit)
    0x0002,
    0x8, // Clocking (2)
    0x01c0,
    0x9, // Audio interface
    0x0000,
    0xa,
    0x01ff,
    0xb,
    0x01ff,
    // Reserved
    0x10,
    0x0000,
    0x11,
    0x007b,
    0x12,
    0x0100,
    0x13,
    0x0032,
    0x14,
    0x0000,
    0x15,
    0x00c3,
    0x16,
    0x00c3,
    0x17,
    0x01c0,
    0x18,
    0x0000,
    0x19,
    0x0140,
    0x1a,
    0x0198,
    0x1b,
    0x0040,
    0x1c,
    0x0000,
    0x1d,
    0x0000,

    0x20,
    0x0100,
    0x21,
    0x0100,
    0x22,
    0x0150,

    0x25,
    0x0150,
    0x26,
    0x0000,
    0x27,
    0x0000,
    0x28,
    0x01ff,
    0x29,
    0x01ff,
    0x2a,
    0x0040,
    0x2b,
    0x0000,
    0x2c,
    0x0000,
    0x2d,
    0x0050,
    0x2e,
    0x0050,
    0x2f,
    0x000c,
    0x30,
    0x0002,
    0x31,
    0x00f7,

    0x33,
    0x0080,
    0x34,
    0x0008,
    0x35,
    0x0031,
    0x36,
    0x0026,
    0x37,
    0x00e9,

};

esp_err_t wm8960_init()
{
  esp_err_t ret = 0;

  wm8960_i2c_init();
  for (int i = 0; i < sizeof(wm8960_reg_defaults) / sizeof(uint16_t); i += 2)
  {
    // printf("[%d] %x %x\n", i, wm8960_reg_defaults[i], wm8960_reg_defaults[i + 1]);
    ret = write_register_i2c(CODEC_ADDR, wm8960_reg_defaults[i], wm8960_reg_defaults[i + 1]);
    if (ret != 0)
    {
      ESP_LOGE(TAG, "Error: %d", ret);
      return ret;
    }
  }

  /*
    ret |= write_register_i2c(CODEC_ADDR, 0x00, 0x00);    // Reset
    ret |= write_register_i2c(CODEC_ADDR, 0x1, 0x3b);     // Power Management 1
    ret |= write_register_i2c(CODEC_ADDR, 0x2, 0x1bf);    // Power Management 2
    ret |= write_register_i2c(CODEC_ADDR, 0x3, 0x6f);     // Power Management 3
    ret |= write_register_i2c(CODEC_ADDR, 0x4, 0x011);     // Configure according to word length and stereo
    ret |= write_register_i2c(CODEC_ADDR, 0x5, 0x00);     // No passthrough
    ret |= write_register_i2c(CODEC_ADDR, 0x6, 0x4c);     // Slave mode
    ret |= write_register_i2c(CODEC_ADDR, 0x7, 0x00);     // 0x00 - 48KHz filter ; 0x06 - 16KHz filter
    ret |= write_register_i2c(CODEC_ADDR, 0x0a, 0x0);     //DAC control - no SW/auto mute, no oversampling, normal polarities
    ret |= write_register_i2c(CODEC_ADDR, 0xb, 0x100 | 220);  // Left DAC volume
    ret |= write_register_i2c(CODEC_ADDR, 0xc, 0x100 | 220);  // Right DAC volume
    ret |= write_register_i2c(CODEC_ADDR, 0xe, 0x8);      //oversampling for better SNR
    ret |= write_register_i2c(CODEC_ADDR, 0xf, 0x8);      //Left ADC volume
    ret |= write_register_i2c(CODEC_ADDR, 0x10, 0x8);     //Right ADC volume
    ret |= write_register_i2c(CODEC_ADDR, 0x2f, 0x100);   //Left PGA boost
    ret |= write_register_i2c(CODEC_ADDR, 0x30, 0x100);   //Right PGA boost
    ret |= write_register_i2c(CODEC_ADDR, 0x2c, 0x33);    //microphones control (device,go through PGA)
    ret |= write_register_i2c(CODEC_ADDR, 0x2d, 0x110);   //Left PGA gain
    ret |= write_register_i2c(CODEC_ADDR, 0x2e, 0x110);   //Right PGA gain
    ret |= write_register_i2c(CODEC_ADDR, 0x34, 0x9);     //Set left HP volume
    ret |= write_register_i2c(CODEC_ADDR, 0x32, 0x15);    //Left Bypass path control from LADC to LMAIN
    ret |= write_register_i2c(CODEC_ADDR, 0x33, 0x15);    //Right Bypass path control from RADC to RMAIN
    */
  if (ret == ESP_OK)
  {
    ESP_LOGE(TAG, "WM8960 is up");
  }
  else
  {
    ESP_LOGE(TAG, "WM8960 setup error");
  }
  return ret;
}

esp_err_t wm8960_set_vol(int vol)
{
  esp_err_t ret = 0;
  int vol_to_set = 0;
  if (vol == 0)
  {
    vol_to_set = 0;
  }
  else
  {
    volume = vol;
    vol_to_set = vol;
  }
  ret |= write_register_i2c(CODEC_ADDR, 0xa, vol_to_set);
  if (ret != 0)
    ESP_LOGE(TAG, "Error: %s", esp_err_to_name(ret));
  ret |= write_register_i2c(CODEC_ADDR, 0xb, 0x100 | vol_to_set);
  if (ret != 0)
    ESP_LOGE(TAG, "Error: %s", esp_err_to_name(ret));

  return ret;
}

esp_err_t wm8960_set_mute(bool mute)
{
  esp_err_t ret = 0;
  if (mute)
  {
    ret |= wm8960_set_vol(0);
  }
  else
  {
    ret |= wm8960_set_vol(volume);
  }
  return ret;
}

esp_err_t wm8960_get_volume(uint8_t *vol)
{
  *vol = volume;
  return ESP_OK;
}
