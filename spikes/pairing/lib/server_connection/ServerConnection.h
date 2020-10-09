#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

class ServerConnection
{

public:
    static void loop_forever();

private:
    char name[14];
    char topic_state[30];
    char topic_settings[35];

    String ssid;
    String password;
    String serverIP;

    ServerConnection();
    void loop();
    static void loop(void*);
    static void handleMqttMessage(char*, byte*, unsigned int);
};
