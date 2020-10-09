#ifndef WM8960_H
#define WM8960_H

#include <Arduino.h>

#define I2C_SDA 14
#define I2C_SCL 27

#define I2S_CLK 26
#define I2S_WS 25
#define I2S_TXSDA 33

class _WM8960 {
public:
    uint8_t begin();
    uint8_t writeRegister(uint8_t reg, uint16_t data);
    uint16_t readRegister(uint8_t reg);

private:
    static const uint8_t WM8960_ADDRESS = 0x1a;
    static uint16_t _register_values[56];
};

extern _WM8960 WM8960;

#endif