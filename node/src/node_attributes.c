#include "node_attributes.h"

void setNodeId() {
  uint64_t chipmacid;
  ESP_ERROR_CHECK(esp_efuse_mac_get_default((uint8_t *)(&chipmacid)));
  node_id = (uint32_t)(chipmacid >> 16);
}

void setNodeIpAddress(const char *ip_address) {
  memcpy(node_ip_address, ip_address, INET_ADDRSTRLEN);
}

void setMqttServerUri(const char *server_ip_address) {
  snprintf(mqtt_server_uri, 7 + INET_ADDRSTRLEN, "mqtt://%s", server_ip_address);
}

