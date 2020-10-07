#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/ringbuf.h>
#include <esp_wifi.h>
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "driver/i2s.h"

RingbufHandle_t buffer;

static EventGroupHandle_t s_wifi_event_group;
static const int CONNECTED_BIT = BIT0;

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

void udpTask(void *params)
{
  char rx[500];

  for (;;)
  {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(2137);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
      ESP_LOGE("udp", "Unable to create socket: errno %d", errno);
      break;
    }
    ESP_LOGI("udp", "Socket created");

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
      ESP_LOGE("udp", "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI("udp", "Socket bound, port 2137");

    for (;;)
    {
      ESP_LOGI("udp", "Waiting for data");
      int len = recvfrom(sock, rx, 500, 0, NULL, NULL);

      if (len < 0)
      {
        ESP_LOGE("udp", "recvfrom failed: errno %d", errno);
        break;
      }
      else
      {
        ESP_LOGI("udp", "recvfrom success");
        xRingbufferSend(buffer, rx, 500, 100);
      }
    }
  }
  vTaskDelete(NULL);
}

void i2sTask(void *params)
{
  size_t bytes_written;
  uint8_t tx_blank[500];
  memset(tx_blank, 0, 500);

  for (;;)
  {
    uint8_t *tx = (uint8_t *)xRingbufferReceive(buffer, &bytes_written, 10);
    if (tx == NULL)
    {
      // i2s_write(I2S_NUM_0, tx_blank, 500, &bytes_written, portMAX_DELAY);
      continue;
    }
    // i2s_write(I2S_NUM_0, tx, 500, &bytes_written, portMAX_DELAY);

    vRingbufferReturnItem(buffer, tx);
  }
}

void waitingTask(void *params)
{
  EventBits_t uxBits;

  for (;;)
  {
    ESP_LOGI("wait", "WiFi Connecting");
    uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, true, false, portMAX_DELAY);
    if (uxBits & CONNECTED_BIT)
    {
      ESP_LOGI("wait", "WiFi Connected to ap");
      xTaskCreate(udpTask, "udp_server", 4096, NULL, 5, NULL);
      xTaskCreate(i2sTask, "i2s_task", 4096, NULL, 5, NULL);
      vTaskDelete(NULL);
    }
  }
}
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    xTaskCreate(waitingTask, "waitingTask", 4096, NULL, 3, NULL);
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    esp_wifi_connect();
    xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
  }
}

void app_main()
{
  xTaskCreate(memTask, "memTask", 4096, NULL, 5, NULL);
  buffer = xRingbufferCreate(500 * 260, RINGBUF_TYPE_BYTEBUF);
  if (buffer == NULL)
  {
    ESP_LOGE("buffer", "NULL");
    abort();
  }
  else
  {
    ESP_LOGI("buffer", "%u", xRingbufferGetCurFreeSize(buffer));
  }

  ESP_ERROR_CHECK(nvs_flash_init());

  ESP_ERROR_CHECK(esp_netif_init());
  s_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  assert(sta_netif);

  wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  wifi_config_t sta_config = {
      .sta = {
          .ssid = "...",
          .password = "..."},
  };
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());
}
