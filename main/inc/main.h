#ifndef MAIN_H
#define MAIN_H
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

#include "wifi_setup.h"
#include "received_messages.h"
#include "firmware_update.h"
#include "mqtt_stuff.h"


#define LED_PIN 2

static const char *TAG = "MAIN"; // TAG for debug

#endif