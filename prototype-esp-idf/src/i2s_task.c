#include "i2s_task.h"

void i2sTask(RingbufHandle_t buffer)
{
  // RingbufHandle_t buffer = (RingbufHandle_t)ringbuf_handle;

  size_t bytes_written;
  uint8_t tx_blank[I2S_BUFFER_SIZE];
  memset(tx_blank, 0, I2S_BUFFER_SIZE);

  for (;;)
  {
    uint8_t *tx = (uint8_t *)xRingbufferReceive(buffer, &bytes_written, 10);
    if (tx == NULL)
    {
      i2s_write(I2S_NUM_0, tx_blank, I2S_BUFFER_SIZE, &bytes_written, portMAX_DELAY);
      continue;
    }
    i2s_write(I2S_NUM_0, tx, I2S_BUFFER_SIZE, &bytes_written, portMAX_DELAY);

    vRingbufferReturnItem(buffer, tx);
  }
}
