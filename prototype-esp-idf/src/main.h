#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>

#include "wm8960.h"
#include "i2s_setup.h"

#include "ring_buffer.h"

#include "memory_debug_task.h"
#include "wifi_task.h"
#include "udp_task.h"
#include "i2s_task.h"
#include "mqtt_task.h"

#ifndef MAIN_H
#define MAIN_H

static void onWifiConnected();

#endif
