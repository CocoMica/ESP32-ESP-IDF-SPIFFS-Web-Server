#ifndef WEB_SERVER_H
#define WEB_SERVER_H
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "spi_flash_mmap.h"
#include <esp_http_server.h>
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include <sys/stat.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "received_messages.h"
#include "storage.h"


extern httpd_uri_t uri_get_1;
extern httpd_uri_t uri_post;
extern httpd_uri_t uri_post_AP_Info;

esp_err_t get_handler(httpd_req_t *req);
esp_err_t post_handler(httpd_req_t *req);

#endif