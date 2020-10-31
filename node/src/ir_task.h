#include <driver/rmt.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <freertos/task.h>
#include "mqtt_client.h"
#include "mqtt_task.h"
#include "ir/tools.h"
#include "node_attributes.h"

#ifndef IR_TASK_H
#define IR_TASK_H

#define IR_TASK_TAG "irTask"
#define IR_CHANNEL_NUM (RMT_CHANNEL_0)

/*
Creates a task that reads signals from an infrared sensor.
@return The created task handle
*/
TaskHandle_t createIrTask();

#endif

