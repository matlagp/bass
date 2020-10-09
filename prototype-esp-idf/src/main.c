#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/ringbuf.h>
#include <esp_wifi.h>
#include <soc/efuse_reg.h>
#include "nvs_flash.h"

#include "wm8960.h"
#include "i2s_setup.h"

#include "wifi_task.h"
// #include "udp_task.h"
// #include "i2s_task.h"
// #include "mqtt_task.h"

// RingbufHandle_t buffer;

void memory_report()
{
  ESP_LOGI("mem", "heap: %d", xPortGetFreeHeapSize());
  ESP_LOGI("mem", "mheap: %d", xPortGetMinimumEverFreeHeapSize());
  ESP_LOGI("mem", "bheap: %u", heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
}

void memTask(void *params)
{
  for (;;)
  {
    memory_report();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void app_main()
{
  xTaskCreate(memTask, "memTask", 4096, NULL, 5, NULL);
  // buffer = xRingbufferCreate(500 * 260, RINGBUF_TYPE_BYTEBUF);
  // if (buffer == NULL)
  // {
  //   ESP_LOGE("buffer", "NULL");
  //   abort();
  // }
  // else
  // {
  //   ESP_LOGI("buffer", "%u", xRingbufferGetCurFreeSize(buffer));
  // }

  createWifiTask();

  wm8960_init();
  wm8960_set_vol(255);

  init_i2s();
}
