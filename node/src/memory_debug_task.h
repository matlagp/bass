#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifndef MEMORY_DEBUG_TASK
#define MEMORY_DEBUG_TASK

#define MEMORY_DEBUG_TASK_TAG "memoryDebug"

/*
When log level is set to INFO or above creates a task that logs heap info every second.
@return The task handle
*/
xTaskHandle createMemoryDebugTask();

#endif
