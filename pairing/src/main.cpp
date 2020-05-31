#include <Arduino.h>
#include <BluetoothSerial.h>

BluetoothSerial bluetoothSerial;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  bluetoothSerial.begin("Node-");

  uint32_t chipId = (uint32_t) ESP.getEfuseMac();
  Serial.printf("%u\n", chipId);
}

void loop() {
  if (bluetoothSerial.available()) {
    String data = bluetoothSerial.readStringUntil('\n');
    switch (data[0]) {
    case '1':
      Serial.printf("SSID: ");
      break;
    case '2':
      Serial.printf("PASS: ");
      break;
    case '3':
      Serial.printf("IP:   ");
      break;
    default:
      Serial.printf("WHAT: ");
    }
    Serial.println(data);
  }

  delay(50);
}