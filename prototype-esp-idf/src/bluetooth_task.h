#include <esp_err.h>
#include <esp_log.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_bt_device.h>
#include <esp_spp_api.h>
#include "lwip/sockets.h"

#include "time.h"
#include "sys/time.h"

#ifndef BLUETOOTH_TASK_H
#define BLUETOOTH_TASK_H

#define BLUETOOTH_TASK_TAG "btTask"

void createBluetoothTask(void (*on_credentials_received)(void));

#endif
