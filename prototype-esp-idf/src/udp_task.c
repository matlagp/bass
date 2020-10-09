#include "udp_task.h"

static void udpTask(RingbufHandle_t buffer);

TaskHandle_t createUdpTask(RingbufHandle_t buffer)
{
  xTaskHandle xHandle = NULL;

  xTaskCreate(udpTask, UDP_TASK_TAG, 4096, buffer, 5, &xHandle);
  if (xHandle == NULL)
  {
    ESP_LOGE(UDP_TASK_TAG, "Could not create task");
    abort();
  }
  return xHandle;
}

static void udpTask(RingbufHandle_t buffer)
{
  char rx[UDP_BUFFER_SIZE];

  for (;;)
  {
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
      ESP_LOGE(UDP_TASK_TAG, "Unable to create socket: errno %d", errno);
      abort();
    }
    ESP_LOGI(UDP_TASK_TAG, "Socket created");

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
      ESP_LOGE(UDP_TASK_TAG, "Socket unable to bind: errno %d", errno);
      abort();
    }
    ESP_LOGI(UDP_TASK_TAG, "Socket bound, port %d", UDP_PORT);

    for (;;)
    {
      ESP_LOGI(UDP_TASK_TAG, "Waiting for data");
      int len = recvfrom(sock, rx, UDP_BUFFER_SIZE, 0, NULL, NULL);

      if (len < 0)
      {
        ESP_LOGE(UDP_TASK_TAG, "recvfrom failed: errno %d", errno);
      }
      else
      {
        ESP_LOGI(UDP_TASK_TAG, "recvfrom success");
        xRingbufferSend(buffer, rx, len, 100);
      }
    }
  }
}
