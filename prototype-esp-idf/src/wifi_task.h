#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include "lwip/sockets.h"

#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#define WIFI_TASK_TAG "wifiTask"

void createWifiTask();

#endif
