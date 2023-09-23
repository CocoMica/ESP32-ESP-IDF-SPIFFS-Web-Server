#ifndef URL_STUFF_H
#define URL_STUFF_H
#define LED_PIN 2

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
#include "web_server.h"

struct async_resp_arg
{
    httpd_handle_t hd;
    int fd;
};

extern int led_state;
extern char *send_message;
extern httpd_handle_t server;

void ws_async_send(void *arg);
void setup_websocket_server(void);
esp_err_t script_js_handler(httpd_req_t *req);
esp_err_t styles_css_handler(httpd_req_t *req);
esp_err_t send_web_page(httpd_req_t *req);
esp_err_t get_req_handler(httpd_req_t *req);
esp_err_t led_on_handler(httpd_req_t *req);
esp_err_t led_off_handler(httpd_req_t *req);
httpd_handle_t setup_server(void);
// esp_err_t trigger_async_send(char *sendMsg);

#endif