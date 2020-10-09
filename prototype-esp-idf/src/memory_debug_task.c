#include "memory_debug_task.h"
#include "esp_log.h"

static void memoryDebugTask(void *);

xTaskHandle createMemoryDebugTask()
{
  if (CONFIG_LOG_DEFAULT_LEVEL >= 3)
  {
    ESP_LOGI(MEMORY_DEBUG_TASK_TAG, "Creating memory debug task");

    xTaskHandle xHandle = NULL;
    xTaskCreate(memoryDebugTask, MEMORY_DEBUG_TASK_TAG, 4096, NULL, 5, &xHandle);
    if (xHandle == NULL)
    {
      ESP_LOGE(MEMORY_DEBUG_TASK_TAG, "Could not create task");
      abort();
    }
    return xHandle;
  }
  else
  {
    return NULL;
  }
}

static void memoryDebugTask(void *_)
{
  for (;;)
  {
    ESP_LOGI(MEMORY_DEBUG_TASK_TAG, "heap: %d", xPortGetFreeHeapSize());
    ESP_LOGI(MEMORY_DEBUG_TASK_TAG, "mheap: %d", xPortGetMinimumEverFreeHeapSize());
    ESP_LOGI(MEMORY_DEBUG_TASK_TAG, "bheap: %u", heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
