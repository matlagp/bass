#include <ServerConnection.h>
#include <Arduino.h>
#include <FreeRTOS.h>
#include <Settings.h>
#include <WiFi.h>

ServerConnection::ServerConnection()
{
  snprintf(name, 14, "node-%08X", (uint32_t)ESP.getEfuseMac());
  snprintf(topic_state, 30, "/nodes/%08X/state", (uint32_t)ESP.getEfuseMac());
  snprintf(topic_settings, 35, "/nodes/%08X/settings/#", (uint32_t)ESP.getEfuseMac());
}

void ServerConnection::loop_forever()
{
  xTaskCreate(loop, "Connect", 2 << 12, NULL, 2, NULL);
}

void ServerConnection::loop(void *params)
{
  ServerConnection sc;
  sc.loop();
}

void ServerConnection::loop()
{
  BluetoothSerial bluetoothSerial;

  WiFiClient wifiClient;
  PubSubClient pubSubClient(wifiClient);

  int retries = 0;
  int setupStep = 0;

  Serial.println(name);
  bluetoothSerial.begin(name);

  while (true)
  {
    // Reading WiFi config from Bluetooth
    while (setupStep < 3 && bluetoothSerial.available())
    {
      char type = bluetoothSerial.read();
      String data = bluetoothSerial.readStringUntil('\n');
      if (type == '1')
      {
        Serial.printf("SSID: ");
        ssid = data;
        setupStep = 1;
      }
      else if (setupStep == 1 && type == '2')
      {
        Serial.printf("PASS: ");
        password = data;
        setupStep = 2;
      }
      else if (setupStep == 2 && type == '3')
      {
        Serial.printf("IP:   ");
        serverIP = data;
        setupStep = 3;
      }
      else
      {
        Serial.printf("WHAT: ");
      }

      Serial.println(data);

      if (setupStep == 3)
      {
        Serial.println("Connecting to WiFi");
        bluetoothSerial.disconnect();

        WiFi.begin(ssid.c_str(), password.c_str());
        retries = 200;
      }
    }

    // Connecting to WiFi
    if (setupStep == 3)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        retries--;
      }
      else
      {
        setupStep = 4;
        Serial.println("Connected to WiFi");
        Serial.println(WiFi.localIP());
        bluetoothSerial.end();

        pubSubClient.setServer(serverIP.c_str(), 1883);
        pubSubClient.setCallback(handleMqttMessage);
      }
    }

    // Back to BT
    if (setupStep == 3 && retries < 0)
    {
      Serial.println("Connecting to WiFi failed");
      setupStep = 0;
    }

    // Connecting to MQTT
    if (setupStep == 4 && !pubSubClient.connected() && pubSubClient.connect(name, topic_state, 0, true, "0"))
    {
      Serial.println("Connected to MQTT");
      pubSubClient.subscribe(topic_settings);
      pubSubClient.publish(topic_state, WiFi.localIP().toString().c_str(), true);
      setupStep = 5;
    }

    // Handling messages and MQTT reconnecting
    if (setupStep == 5 && !pubSubClient.loop())
    {
      setupStep = 4;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void ServerConnection::handleMqttMessage(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  char msg_c_string[4] = "\0\0\0";

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
  }
  Serial.println();

  int j = 0;
  for (int i = 0; topic[i] != '\0'; i++)
  {
    if (topic[i] == '/')
      j = i + 1;
  }

  if (strcmp(&topic[j], "volume") == 0)
  {
    strncpy(msg_c_string, (char *)message, length > 3 ? 3 : length);
    int newVolume = atoi(msg_c_string);
    if (newVolume >= 0 && newVolume <= 100)
    {
      Settings &settings = Settings::getInstance();
      settings.setVolume(newVolume);
      Serial.printf("Volume: %d\n", settings.getVolume());
    }
    else
    {
      Serial.printf("Volume: %d is invalid\n", newVolume);
    }
  }
}
