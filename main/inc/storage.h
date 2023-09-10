#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
#include <string.h>
#include "esp_spiffs.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "wifi_setup.h"

#define TEMP_FILE_PATH "/storage/myfile.txt"
#define NVS_WIFI_INFO "nvs_wifi_info"
typedef struct Wifi_Info_t
{
    char STA_ssid[32];
    uint8_t STA_ssid_len;
    char STA_password[64];
    uint8_t STA_password_len;

    char AP_ssid[32];
    uint8_t AP_ssid_len;
    char AP_password[64];
    uint8_t AP_password_len;
    uint8_t AP_IP[4];//192 168 2 1
    uint8_t AP_GW[4];//192 168 2 1
    uint8_t AP_netmask[4];//255 255 255 0

    uint8_t max_retries;

} __attribute__((packed)) Wifi_Info_t;

typedef struct NVS_DATA_t
{
    Wifi_Info_t wifi_info;
    int num1;
    int num2;
    uint8_t led_state;
    char character;
    char buffer[50];
} __attribute__((packed)) NVS_DATA_t;

void init_storage();
Wifi_Info_t nvs_get_wifi_information(bool print_out_information);
esp_err_t nvs_set_wifi_information(Wifi_Info_t new_wifi_info);

#endif