#include "ring_buffer.h"

RingbufHandle_t createRingBuffer()
{
  size_t buffer_size = RING_BUFFER_ITEM_SIZE * RING_BUFFER_ITEM_COUNT;
  RingbufHandle_t buffer = xRingbufferCreate(buffer_size, RINGBUF_TYPE_BYTEBUF);

  if (buffer == NULL)
  {
    ESP_LOGE(RING_BUFFER_TAG, "Could not create ring buffer");
    abort();
  }
  else
  {
    ESP_LOGI(RING_BUFFER_TAG, "Ring buffer of size %uB created", xRingbufferGetCurFreeSize(buffer));
  }

  return buffer;
}
