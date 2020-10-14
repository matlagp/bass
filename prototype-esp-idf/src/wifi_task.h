#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "lwip/sockets.h"

#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#define WIFI_TASK_TAG "wifiTask"

/*
Initializes WiFi and creates a task that handles the connection.

@param on_connected Function called after WiFi connects for the first time.
@param on_disconnected Function called after WiFi disconnects.
@param on_reconnected Function called after subsequent WiFi connections.
*/
void createWifiTask(
    void (*on_connected)(char *ip_address),
    void (*on_disconnected)(void),
    void (*on_reconnected)(char *ip_address));

#endif
