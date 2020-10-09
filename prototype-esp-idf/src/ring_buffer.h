#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define RING_BUFFER_TAG "ringbuffer"

#define RING_BUFFER_ITEM_SIZE 500
#define RING_BUFFER_ITEM_COUNT 260

RingbufHandle_t createRingBuffer();

#endif
