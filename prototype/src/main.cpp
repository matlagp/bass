#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "ServerConnection.h"

#include "FreeRTOS.h"
#include "wm8960.h"
#include "i2s_setup.h"
#include "driver/i2s.h"
#include "freertos/ringbuf.h"
#include "lwip/sockets.h"

RingbufHandle_t buffer;

void dataIntakeTask(void *);

void tcpServerTask(void *pvParameters)
{
  // byte rxBuffer[512];
  sockaddr_in localAddress;

  // Configure IPv4 address
  localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddress.sin_family = AF_INET;
  localAddress.sin_port = htons(2137);

  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (sock < 1)
  {
    Serial.printf("Unable to create socket, error code: %d\n", errno);
    vTaskDelete(NULL);
    return;
  }
  Serial.println("Socket created successfully");

  if (bind(sock, (sockaddr *)&localAddress, sizeof(localAddress)) < 0)
  {
    Serial.printf("Unable to bind socket, error code: %d\n", errno);
    vTaskDelete(NULL);
    return;
  }
  Serial.println("Socket bound successfully");

  if (listen(sock, 2) != 0)
  {
    Serial.printf("Unable to listen on socket, error code: %d\n", errno);
    vTaskDelete(NULL);
    return;
  }
  Serial.println("Socket listening");

  while (true)
  {
    sockaddr_in remoteAddress;
    socklen_t socklen = sizeof(remoteAddress);

    int *remoteSock = new int;
    *remoteSock = accept(sock, (sockaddr *)&remoteAddress, &socklen);
    if (*remoteSock < 0)
    {
      Serial.printf("Unable to accept connection, error code: %d\n", errno);
      continue;
    }
    Serial.println("Got connection");

    xTaskCreate(dataIntakeTask, "DataIntake", 1 << 13, (void *)remoteSock, 3,
                NULL);

    // int len = recvfrom(sock, rxBuffer, sizeof(rxBuffer), 0,
    //                    (sockaddr *)&remoteAddress, &socklen);

    // if (len < 0) {
    //   Serial.printf("recvfrom failed, error code: %d\n", errno);
    //   continue;
    // }
    // Serial.printf("Received %d bytes\n", len);

    // Send received data to the ring buffer
    // while (xRingbufferSend(buffer, rxBuffer, len, pdMS_TO_TICKS(100)) !=
    //        pdTRUE) {
    //   // Serial.println("Failed to write bytes to buffer");
    //   vTaskDelay(pdMS_TO_TICKS(100));
    // }
  }

  vTaskDelete(NULL);
}

void dataIntakeTask(void *pvParameters)
{
  int *remoteSock = (int *)pvParameters;
  byte rxData[4096];

  while (true)
  {
    int len = recv(*remoteSock, rxData, sizeof(rxData), 0);

    if (len < 0)
    {
      Serial.println("Error while receiving network data");
      continue;
    }

    if (len == 0)
    {
      Serial.println("Connection closed");
      close(*remoteSock);
      delete remoteSock;
      break;
    }

    // Send received data to the ring buffer
    while (xRingbufferSend(buffer, rxData, len, pdMS_TO_TICKS(100)) != pdTRUE)
    {
      // Serial.println("Failed to write bytes to buffer");
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  vTaskDelete(NULL);
}

void i2sTask(void *pvParameters)
{
  while (true)
  {
    size_t readSize;
    byte *data = (byte *)xRingbufferReceiveUpTo(buffer, &readSize,
                                                pdMS_TO_TICKS(100), 4096);

    if (data != NULL)
    {
      // Serial.printf("Read %d bytes:\n", readSize);
      // for (int i = 0; i < readSize; i++) {
      //   printf("%c", data[i]);
      // }
      // printf("\n");

      size_t writtenSize = 0;
      while (writtenSize < readSize)
      {
        esp_err_t err =
            i2s_write(I2S_NUM_0, data + writtenSize, readSize - writtenSize,
                      &writtenSize, pdMS_TO_TICKS(100));
        // Serial.printf("Wrote %d bytes so far\n", writtenSize);
        if (err)
        {
          Serial.printf("Error while pushing data to DAC: %d\n", err);
        }
      }

      vRingbufferReturnItem(buffer, (void *)data);
    }
    else
    {
      // Serial.println("Failed to read data");
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}

void setup()
{
  Serial.begin(9600);

  wm8960_init();
  wm8960_set_vol(255);

  buffer = xRingbufferCreate(1 << 14, RINGBUF_TYPE_BYTEBUF);
  xRingbufferPrintInfo(buffer);

  init_i2s();

  ServerConnection::loop_forever();

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Not connected yet");
    delay(1000);
  }
  Serial.println("Connected");

  xTaskCreate(tcpServerTask, "Server", 4096, NULL, 2, NULL);
  xTaskCreate(i2sTask, "I2S", 8192, NULL, 3, NULL);
}

void loop()
{
  while (true)
  {
    // Serial.println("Loop task");
    delay(1000);
  }
}