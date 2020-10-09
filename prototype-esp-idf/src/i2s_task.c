#include "i2s_task.h"

static void i2sTask(RingbufHandle_t buffer);

TaskHandle_t createI2sTask(RingbufHandle_t buffer)
{
  xTaskHandle xHandle = NULL;

  xTaskCreate(i2sTask, I2S_TASK_TAG, 4096, buffer, 5, &xHandle);
  if (xHandle == NULL)
  {
    ESP_LOGE(I2S_TASK_TAG, "Could not create task");
    abort();
  }
  return xHandle;
}

static void i2sTask(RingbufHandle_t buffer)
{
  size_t _bytes_written;
  uint8_t tx_blank[I2S_BUFFER_SIZE];
  memset(tx_blank, 0, I2S_BUFFER_SIZE);

  for (;;)
  {
    uint8_t *tx = (uint8_t *)xRingbufferReceive(buffer, &_bytes_written, 10);
    if (tx == NULL)
    {
      i2s_write(I2S_PORT_NUM, tx_blank, I2S_BUFFER_SIZE, &_bytes_written, portMAX_DELAY);
      continue;
    }
    i2s_write(I2S_PORT_NUM, tx, I2S_BUFFER_SIZE, &_bytes_written, portMAX_DELAY);

    vRingbufferReturnItem(buffer, tx);
  }
}
