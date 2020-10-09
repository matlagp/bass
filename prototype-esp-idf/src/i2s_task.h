#include <driver/i2s.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>
#include <string.h>

#ifndef I2S_TASK_H
#define I2S_TASK_H

#define I2S_BUFFER_SIZE 500

/*
Creates a task that copies samples from a given ringbuf to the I2S driver.
*/
void i2sTask(RingbufHandle_t buffer);

#endif
