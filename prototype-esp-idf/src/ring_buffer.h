#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/ringbuf.h>

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define RING_BUFFER_TAG "ringbuffer"

#define RING_BUFFER_ITEM_SIZE 512
#define RING_BUFFER_ITEM_COUNT 200

RingbufHandle_t createRingBuffer();

#endif
