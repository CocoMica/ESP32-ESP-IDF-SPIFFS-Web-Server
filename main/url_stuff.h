#ifndef URL_STUFF_H
#define URL_STUFF_H

//#include <esp_http_server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //Requires by memset
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
extern int led_state;

void initi_web_page_buffer(void);

esp_err_t script_js_handler(httpd_req_t *req);
esp_err_t styles_css_handler(httpd_req_t *req);
esp_err_t send_web_page(httpd_req_t *req);
esp_err_t get_req_handler(httpd_req_t *req);
esp_err_t led_on_handler(httpd_req_t *req);
esp_err_t led_off_handler(httpd_req_t *req);

httpd_handle_t setup_server(void);

#endif // URL_STUFF_H