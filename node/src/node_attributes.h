#include "lwip/sockets.h"

#ifndef NODE_ATTRIBUTES_H
#define NODE_ATTRIBUTES_H

uint32_t node_id;
char node_ip_address[INET_ADDRSTRLEN];
char mqtt_server_uri[7 + INET_ADDRSTRLEN];

void setNodeId();
void setNodeIpAddress(const char *ip_address);
void setMqttServerUri(const char *server_ip_address);

#endif
