#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
#include <string.h>
#include "esp_spiffs.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "wifi_setup.h"
#include "mqtt_client.h"

#define SUBMIT_HTML_PATH "/spiffs/submit.html"
#define INDEX_HTML_PATH "/spiffs/index.html"
#define INDEX_JS_PATH "/spiffs/script.js"
#define STYLES_CSS_PATH "/spiffs/styles.css"
#define IMAGE_PATH "/spiffs/test.png"

#define TEMP_FILE_PATH "/storage/myfile.txt"
#define NVS_WIFI_INFO "nvs_wifi_info"
#define NVS_GENERAL_INFO "nvs_gen_info"
#define NVS_MQTT_INFO "nvs_mqtt_info"

#define CONFIG_BROKER_BIN_SIZE_TO_SEND 20000


extern char NVS_Submit_HTML[4096];
extern char NVS_Index_HTML[4096];
extern char NVS_Index_JS[4096];
extern char NVS_Styles_CSS[4096];
extern char response_data[4096];
extern char NVS_Image_Test[25000];


typedef struct Wifi_Info_t
{
    esp_netif_ip_info_t ipInfo;
    char STA_ssid[32];
    uint8_t STA_ssid_len;
    char STA_password[64];
    uint8_t STA_password_len;
    char AP_ssid[32];
    uint8_t AP_ssid_len;
    char AP_password[64];
    uint8_t AP_password_len;
    uint8_t AP_IP[4];
    uint8_t AP_GW[4];
    uint8_t AP_netmask[4];
    uint8_t max_retries;

} Wifi_Info_t;




typedef struct General_Info_t
{
    esp_reset_reason_t last_reset_reason;

} General_Info_t;

typedef struct Machine_Info_t
{
    General_Info_t general_info;
    Wifi_Info_t wifi_info;


} Machine_Info_t;

typedef struct NVS_DATA_t
{
    Wifi_Info_t wifi_info;
    int num1;
    int num2;
    uint8_t led_state;
    char character;
    char buffer[50];
} __attribute__((packed)) NVS_DATA_t;



extern Machine_Info_t machine_info;

void init_storage();
esp_err_t load_spiffs_pages();
esp_err_t nvs_get_wifi_information(bool print_out_information);
esp_err_t nvs_set_wifi_information(Wifi_Info_t new_wifi_info);
esp_err_t nvs_get_general_information(bool print_out_information);
esp_err_t nvs_set_general_information(General_Info_t new_general_info);

#endif