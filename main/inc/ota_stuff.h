#ifndef OTA_STUFF_H
#define OTA_STUFF_H
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <esp_partition.h>
#include "esp_ota_ops.h"
#include "mqtt_client.h"

extern bool expect_undefined_len_OTA_data;
void handle_mqtt_ota_commands(esp_mqtt_event_handle_t event);
void mqtt_subscribe_ota_functions(esp_mqtt_client_handle_t client);
void mqtt_unsubscribe_ota_functions(esp_mqtt_client_handle_t client);
esp_err_t ota_begin();
esp_err_t ota_write_data(const void *data, size_t size);
esp_err_t ota_end();
void update_ota_data_length_to_be_expected(const char *data);
void process_ota_data(const char *data, size_t data_len);
#endif