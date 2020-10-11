#include <driver/i2s.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <freertos/task.h>
#include <string.h>

#ifndef I2S_TASK_H
#define I2S_TASK_H

#define I2S_TASK_TAG "i2sTask"

#define I2S_PORT_NUM I2S_NUM_0
#define I2S_BUFFER_SIZE 4096

/*
Creates a task that copies samples from a given ringbuf to the I2S driver.
@param buffer Handle to the desired FreeRTOS Ring Buffer
@return The created task handle
*/
TaskHandle_t createI2sTask(RingbufHandle_t buffer);

#endif
