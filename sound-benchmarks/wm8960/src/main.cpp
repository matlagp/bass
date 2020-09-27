#include <Arduino.h>
#include <wm8960.h>
#include <sine.h>

void setup()
{
  Serial.begin(9600);
  wm8960_init();
  init_i2s();
}

int test_bits = 16;

void loop()
{
  setup_triangle_sine_waves(test_bits);
  delay(5000);
  test_bits += 8;
  if (test_bits > 32)
    test_bits = 16;
}