#include <Arduino.h>
#include <ServerConnection.h>

void setup()
{
  Serial.begin(9600);

  ServerConnection::loop_forever();
}

void loop() {}
