#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <freertos/task.h>
#include "lwip/sockets.h"

#ifndef UDP_TASK_H
#define UDP_TASK_H

#define UDP_TASK_TAG "udpTask"

#define UDP_PORT 2137
#define UDP_BUFFER_SIZE 4096

/*
Creates a task that received samples from UDP datagrams and puts them in the given ringbuf.
@param buffer Handle to the desired FreeRTOS Ring Buffer
@return The created task handle
*/
TaskHandle_t createUdpTask(RingbufHandle_t buffer);

#endif
