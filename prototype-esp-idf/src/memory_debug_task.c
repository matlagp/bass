#include "memory_debug_task.h"
#include "esp_log.h"

static void memoryDebugTask(void *);

static uint32_t heap = 0;
static uint32_t mheap = 0;
static size_t bheap = 0;

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
    uint32_t new_heap = xPortGetFreeHeapSize();
    uint32_t new_mheap = xPortGetMinimumEverFreeHeapSize();
    size_t new_bheap = heap_caps_get_largest_free_block(MALLOC_CAP_32BIT);
    if (new_heap != heap || new_mheap != mheap || new_bheap != bheap)
    {
      ESP_LOGI(MEMORY_DEBUG_TASK_TAG, "heap: %u -> %u", heap, new_heap);
      ESP_LOGI(MEMORY_DEBUG_TASK_TAG, "mheap: %u -> %u", mheap, new_mheap);
      ESP_LOGI(MEMORY_DEBUG_TASK_TAG, "bheap: %u -> %u", bheap, new_bheap);

      heap = new_heap;
      mheap = new_mheap;
      bheap = new_bheap;
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
