#include <Arduino.h>
#include <wm8960.h>

void setup()
{
  Serial.begin(9600);
  Serial.println("\nI2C Scanner");
  wm8960_init();
}

void loop()
{
  uint8_t vol = 0;
  while (1)
  {
    wm8960_set_vol(vol);
    uint8_t new_vol;
    wm8960_get_volume(&new_vol);
    Serial.println(new_vol);
    vol++;
  }
}
