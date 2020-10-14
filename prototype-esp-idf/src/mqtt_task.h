#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "lwip/sockets.h"
#include "mqtt_client.h"

#ifndef MQTT_TASK_H
#define MQTT_TASK_H

#define MQTT_TASK_TAG "mqttTask"

/*
Initializes the MQTT client and creates a task that handles the connection.
WiFi disconnects and reconnects have to be handled manually.

@param ip_address The IP address of the node.
*/
TaskHandle_t createMqttTask(char *ip_address);

/*
Disconnect the MQTT client. Call after a WiFi disconnect.
*/

void disconnectMqtt();

/*
Reconnects the MQTT client. Call after a WiFi reconnect.

@param ip_address The IP address of the node.
*/
void reconnectMqtt(char *ip_address);

#endif
