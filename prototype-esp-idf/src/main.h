#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>

#ifdef USE_WM8960
  #include "wm8960.h"
#endif
#include "i2s_setup.h"
#include "ir_setup.h"

#include "ring_buffer.h"

#include "node_attributes.h"
#include "common_init.h"
#include "memory_debug_task.h"
#include "wifi_task.h"
#include "udp_task.h"
#include "i2s_task.h"
#include "mqtt_task.h"
#include "bluetooth_task.h"
#include "ir_task.h"

#ifndef MAIN_H
#define MAIN_H

static void onWifiCredentialsReceived(char *ssid, char *password, char *server_ip);
static void onWifiNotConnected(void);
static void onWifiConnected(char *ip_address);
static void onWifiDisconnected(void);
static void onWifiReconnected(char *ip_address);

#endif
