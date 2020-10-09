#include "WM8960.h"
#include <Arduino.h>
#include <Wire.h>

uint16_t _WM8960::_register_values[] = {  
    0x0097, 0x0097, 0x0000, 0x0000, 0x0000, 0x0008, 0x0000, 0x000A,
    0x01C0, 0x0000, 0x00FF, 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x007B, 0x0100, 0x0032, 0x0000, 0x00C3, 0x00C3, 0x01C0,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0100, 0x0100, 0x0050, 0x0050, 0x0050, 0x0050, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0040, 0x0000, 0x0000, 0x0050, 0x0050, 0x0000,
    0x0000, 0x0037, 0x004D, 0x0080, 0x0008, 0x0031, 0x0026, 0x00ED
};

uint8_t _WM8960::begin() {
    uint8_t res;

    // Reset Device
    res = writeRegister(0x0f, 0x0000);
    if (res != 0) {
        return res;
    }
  
    // Set Power Source
    res =  writeRegister(0x19, 1<<8 | 1<<7 | 1<<6);
    res += writeRegister(0x1A, 1<<8 | 1<<7 | 1<<6 | 1<<5 | 1<<4 | 1<<3);
    res += writeRegister(0x2F, 1<<3 | 1<<2);
    if (res != 0)  {
        return res;
    }
  
    // Configure clock
    // MCLK->div1->SYSCLK->DAC/ADC sample Freq = 25MHz(MCLK)/2*256 = 48.8kHz
    writeRegister(0x04, 0x0000);
    
    // Configure ADC/DAC
    writeRegister(0x05, 0x0000);
    
    // Configure audio interface
    // I2S format 16 bits word length
    writeRegister(0x07, 0x0002);
    
    // Configure HP_L and HP_R OUTPUTS
    writeRegister(0x02, 0x0070 | 0x0100);  // LOUT1 Volume Set
    writeRegister(0x03, 0x0070 | 0x0100);  // ROUT1 Volume Set
    
    // Configure SPK_RP and SPK_RN
    writeRegister(0x28, 0x0000 | 0x0100); // Left Speaker Volume
    writeRegister(0x29, 0x0000 | 0x0100); // Right Speaker Volume
    
    // Enable the OUTPUTS
    writeRegister(0x31, 0x0037); // Disable Class D Speaker Outputs
    
    // Configure DAC volume
    writeRegister(0x0a, 0x00FF | 0x0100);
    writeRegister(0x0b, 0x00FF | 0x0100);
    
    // 3D
    // writeRegister(0x10, 0x001F);
    
    // Configure MIXER
    writeRegister(0x22, 1<<8 | 1<<7);
    writeRegister(0x25, 1<<8 | 1<<7);
    
    // Jack Detect
    writeRegister(0x18, 1<<6 | 0<<5);
    writeRegister(0x17, 0x01C3);
    writeRegister(0x30, 0x0009); // 0x000D, 0x0005
    
    return 0;
}

uint8_t _WM8960::writeRegister(uint8_t reg, uint16_t data) {
    uint8_t res, I2C_Data[2];
    
    I2C_Data[0] = (reg<<1)|((uint8_t)((data>>8)&0x0001));  //RegAddr
    I2C_Data[1] = (uint8_t)(data&0x00FF);                  //RegValue
    
    Wire.beginTransmission(WM8960_ADDRESS);
    Wire.write(I2C_Data, 2);
    res = Wire.endTransmission();

    if(res == I2C_ERROR_OK)
        _register_values[reg] = data;
    
    return res;
}

uint16_t _WM8960::readRegister(uint8_t reg) {
    return _register_values[reg];
}

_WM8960 WM8960 = _WM8960();