#ifndef MQTT_STUFF_H
#define MQTT_STUFF_H
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
//#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"

#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>


#define CONFIG_BROKER_BIN_SIZE_TO_SEND 20000
//#define CONFIG_BROKER_URI "mqtts://mqtt.eclipseprojects.io:8883"
#define CONFIG_BROKER_URI "mqtts://bbcbdbaf.ala.us-east-1.emqxsl.com:8883"

void mqtt_app_start(void);

#endif