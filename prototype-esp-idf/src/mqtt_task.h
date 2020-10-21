#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "lwip/sockets.h"
#include "mqtt_client.h"
#include "node_attributes.h"

#ifndef MQTT_TASK_H
#define MQTT_TASK_H

#define MQTT_TASK_TAG "mqttTask"

/*
Initializes the MQTT client and creates a task that handles the connection.
WiFi disconnects and reconnects have to be handled manually.
*/
TaskHandle_t createMqttTask();

/*
Disconnect the MQTT client. Call after a WiFi disconnect.
*/

void disconnectMqtt();

/*
Reconnects the MQTT client. Call after a WiFi reconnect.
*/
void reconnectMqtt();

#endif
