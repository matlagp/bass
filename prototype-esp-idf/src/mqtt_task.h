#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "mqtt_client.h"

#ifndef MQTT_TASK_H
#define MQTT_TASK_H

#define MQTT_TASK_TAG "mqttTask"

TaskHandle_t createMqttTask();

#endif
