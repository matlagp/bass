#include <esp_err.h>
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

#ifndef PERSISTENT_CREDENTIALS_H
#define PERSISTENT_CREDENTIALS_H

#define CREDENTIALS_TAG "credentials"

char *memory_wifi_ssid;
char *memory_wifi_password;
char *memory_server_ip;

esp_err_t loadPersistedCredentials();
esp_err_t saveCredentials(const char *, const char *, const char *);
void freeCredentialsMemory();

#endif
