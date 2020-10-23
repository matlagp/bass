#include <esp_err.h>
#include <esp_log.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_bt_device.h>
#include <esp_spp_api.h>
#include "lwip/sockets.h"

#ifndef BLUETOOTH_TASK_H
#define BLUETOOTH_TASK_H

#define BLUETOOTH_TASK_TAG "btTask"

/*
Initializes the bluetooth SPP server that receives WiFi credentials.

@param on_credentials_received A function called after successfully receiving WiFi credentials
*/
void createBluetoothTask(void (*on_credentials_received)(char *ssid, char *password, char *ip));

/*
Restarts the SPP server to retrieve WiFi credentials again.
Call after checking that previous credentials are invalid.
*/
void retryBluetooth(void);

/*
Cleans up the whole bluetooth stack.
Call after checking that the retrieved credentials are valid.

The bluetooth SPP server cannot be started again after calling this.
*/
void cleanupBluetooth(void);

#endif
