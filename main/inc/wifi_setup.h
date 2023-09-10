#ifndef WIFI_SETUP_H_
#define WIFI_SETUP_H_

#include <esp_system.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>
#include "url_stuff.h"
#include "received_messages.h"
#include "storage.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

extern int wifi_connect_status;

void wifi_init(void);
void from_ap_to_sta(void);
void from_sta_to_ap(void);

void wifi_task(void *pvParameters);





#endif