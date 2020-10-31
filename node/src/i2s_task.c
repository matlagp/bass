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
  size_t bytes_read;
  size_t _bytes_written;

  for (;;)
  {
    uint8_t *tx = (uint8_t *)xRingbufferReceive(buffer, &bytes_read, 20);
    if (tx == NULL)
    {
      i2s_zero_dma_buffer(I2S_PORT_NUM);
      vTaskDelay(300 / portTICK_PERIOD_MS);
      continue;
    }
    i2s_write(I2S_PORT_NUM, tx, bytes_read, &_bytes_written, portMAX_DELAY);

    vRingbufferReturnItem(buffer, tx);
  }
}
