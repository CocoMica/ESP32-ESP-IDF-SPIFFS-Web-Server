#include "inc/ota_stuff.h"
static const char *TAG = "ota_stuff";
esp_ota_handle_t ota_handle;
uint8_t new_partition = 0;
uint32_t total_received_data_len = 0;
uint32_t targetAddress;
bool first_time = true;
uint32_t data_length = 0;
bool expect_undefined_len_OTA_data = false;
void mqtt_subscribe_ota_functions(esp_mqtt_client_handle_t client)
{
    int msg_id;
    msg_id = esp_mqtt_client_subscribe(client, "/ota/binData", 2);
    ESP_LOGI(TAG, "Subscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(client, "/ota/lenBinData", 1);
    ESP_LOGI(TAG, "Subscribe successful, msg_id=%d", msg_id);
}
void mqtt_unsubscribe_ota_functions(esp_mqtt_client_handle_t client)
{
    int msg_id;
    msg_id = esp_mqtt_client_unsubscribe(client, "/ota/binData");
    ESP_LOGI(TAG, "Unsubscribe successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_unsubscribe(client, "/ota/lenBinData");
    ESP_LOGI(TAG, "Unsubscribe successful, msg_id=%d", msg_id);
}

void handle_mqtt_ota_commands(esp_mqtt_event_handle_t event)
{
    if (event->topic_len == 0)
    {
        if (expect_undefined_len_OTA_data)
        {
            process_ota_data(event->data, event->data_len);
        }
    }
    else
    {
        if (strncmp(event->topic, "/ota/lenBinData", event->topic_len) == 0)
        {
            update_ota_data_length_to_be_expected(event->data);
        }
        else if (strncmp(event->topic, "/ota/binData", event->topic_len) == 0)
        {
            process_ota_data(event->data, event->data_len);
        }
    }
}
// Initialize OTA update process
esp_err_t ota_begin()
{
    const esp_partition_t *active_partition = esp_ota_get_boot_partition();
    const esp_partition_t *update_partition;
    if (strcmp(active_partition->label, "ota_0") == 0)
    {
        ESP_LOGI(TAG, "Current active partition is ota_0. Preparing partition ota_1 to store incoming firmware...");
        new_partition = 1;
        update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    }
    else
    {
        ESP_LOGI(TAG, "Current active partition is factory OR ota_1. Preparing partition ota_0 to store incoming firmware...");
        new_partition = 0;
        update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    }
    if (update_partition == NULL)
    {
        return ESP_FAIL;
    }
    return esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
}

// Write firmware data to OTA partition
esp_err_t ota_write_data(const void *data, size_t size)
{
    return esp_ota_write(ota_handle, data, size);
}

// Finish OTA update and set the updated partition as the boot partition
esp_err_t ota_end()
{
    expect_undefined_len_OTA_data = false;
    if (esp_ota_end(ota_handle) != ESP_OK)
    {
        return ESP_FAIL;
    }
    if (esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL)) != ESP_OK)
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}

void update_ota_data_length_to_be_expected(const char *data)
{
    data_length = strtoul(data, NULL, 10);
    ESP_LOGI(TAG, "OTA data about to receive. total firmware size: %ld", data_length);
    expect_undefined_len_OTA_data = true;
}

void process_ota_data(const char *data, size_t data_len)
{
    if (first_time)
    {
        first_time = false;
        esp_err_t ret1 = ota_begin();
        ESP_LOGI(TAG, "begining the OTA. status of OTA: %s", esp_err_to_name(ret1));
    }
    total_received_data_len += data_len;
    esp_err_t ret2 = ota_write_data(data, data_len);
    ESP_LOGI(TAG, "Target partition: ota_%d: Data len: %d, target address: 0x%08lx, total_received: %ld, writing data status: %s", new_partition, data_len, targetAddress, total_received_data_len, esp_err_to_name(ret2));
    targetAddress += data_len;
    if (total_received_data_len == data_length)
    {
        esp_err_t ret3 = ota_end();
        ESP_LOGI(TAG, "All data received. status of ota: %s", esp_err_to_name(ret3));
        ESP_LOGI(TAG, "Next boot will use ota_%d as the active partition.....", new_partition);
        vTaskDelay(pdMS_TO_TICKS(6000));
        esp_restart();
    }
}