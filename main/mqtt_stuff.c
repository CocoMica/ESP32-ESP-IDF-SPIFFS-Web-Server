#include "inc/mqtt_stuff.h"
#include <esp_partition.h>
#include "esp_ota_ops.h"
static const char *TAG = "mqtt_stuff";

#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t mqtt_eclipseprojects_io_pem_start[] = "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE "\n-----END CERTIFICATE-----";
#else
extern const uint8_t mqtt_eclipseprojects_io_pem_start[] asm("_binary_mqtt_eclipseprojects_io_pem_start");
#endif
extern const uint8_t mqtt_eclipseprojects_io_pem_end[] asm("_binary_mqtt_eclipseprojects_io_pem_end");

uint32_t total_received_data_len = 0;
uint32_t targetAddress;
static esp_ota_handle_t ota_handle;
bool first_time = true;
uint32_t data_length = 0;
uint8_t new_partition = 0;
// Initialize OTA update process
esp_err_t ota_begin()
{   const esp_partition_t *active_partition = esp_ota_get_boot_partition();
    const esp_partition_t *update_partition;
if(strcmp(active_partition->label, "ota_0") == 0){
    ESP_LOGI(TAG, "Current active partition is ota_0. Preparing partition ota_1 to store incoming firmware...");
    new_partition = 1;
    update_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
}else{
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

void copy_data_to_ota_0(const char *data, size_t data_len)
{
    // esp_partition_t *otaPartition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
    total_received_data_len += data_len;
    ESP_LOGI(TAG, "copy_data_to_ota_%d: Data len=%d, copy address:  0x%08lx, total_received=%ld.",new_partition, data_len, targetAddress, total_received_data_len);
    // ESP_LOGW(TAG, "copy_data_to_ota_0: Data len=%d, total_received=%ld. first character: %c: ", data_len,  total_received_data_len, data[0]);

    // memcpy((void*)targetAddress, data, data_len);
    // uint8_t *target_address = (uint8_t *)otaPartition->address;
    // memcpy(otaPartition->address, data, data_len);
    targetAddress += data_len;
    if (first_time)
    {
        first_time = false;
        esp_err_t ret1 = ota_begin();
        ESP_LOGI(TAG, "begining the OTA. status of OTA: %s", esp_err_to_name(ret1));
    }
    esp_err_t ret2 = ota_write_data(data, data_len);
    ESP_LOGI(TAG, "writing data. status: %s", esp_err_to_name(ret2));
    if (total_received_data_len == data_length)
    {
        // end
        esp_err_t ret3 = ota_end();
        ESP_LOGI(TAG, "all data received. status of ota: %s", esp_err_to_name(ret3));
        // Use esp_ota_set_boot_partition() to set ota_0 as the active partition
       // esp_ota_set_boot_partition(esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, ota0_label));
        // Print a message indicating that the next boot will use ota_0
        printf("Next boot will use ota_%d as the active partition.....\n",new_partition);
        vTaskDelay(pdMS_TO_TICKS(6000));
        esp_restart();
    }
}
//
// Note: this function is for testing purposes only publishing part of the active partition
//       (to be checked against the original binary)
//
static void send_binary(esp_mqtt_client_handle_t client)
{
    esp_partition_mmap_handle_t out_handle;
    const void *binary_address;
    const esp_partition_t *partition = esp_ota_get_running_partition();
    esp_partition_mmap(partition, 0, partition->size, ESP_PARTITION_MMAP_DATA, &binary_address, &out_handle);
    // sending only the configured portion of the partition (if it's less than the partition size)
    int binary_size = MIN(CONFIG_BROKER_BIN_SIZE_TO_SEND, partition->size);
    int msg_id = esp_mqtt_client_publish(client, "/topic/binary", binary_address, binary_size, 0, 0);
    ESP_LOGI(TAG, "binary sent with msg_id=%d", msg_id);
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:

        if (strncmp(event->topic, "/topic/qos0", event->topic_len) == 0)
        {
            copy_data_to_ota_0(event->data, event->data_len);
        }
        else if (strncmp(event->topic, "/topic/qos1", event->topic_len) == 0)
        {
           data_length = strtoul(event->data, NULL, 10);
            ESP_LOGI(TAG, "OTA data about to receive. total firmware size: %ld", data_length);
            
        }
        else
        {
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGW(TAG, "Data len=%d", event->data_len);
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
        }
        if (strncmp(event->data, "send binary please", event->data_len) == 0)
        {
            ESP_LOGI(TAG, "Sending the binary");
            send_binary(client);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)", event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        }
        else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED)
        {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        }
        else
        {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "mqtt_app_start");
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = CONFIG_BROKER_URI,
            .verification.certificate = (const char *)mqtt_eclipseprojects_io_pem_start,
        },
        .credentials = {
            .username = "device1",
            .authentication.password = "device001",
        }};
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    int count = 0;
    while (true)
    {
        return;
        vTaskDelay(pdMS_TO_TICKS(3000));
        char data[80];
        count++;
        sprintf(data, "count is: %d", count);
        int msg_id = esp_mqtt_client_publish(client, "/topic/qos0", data, 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d, count=%d.", msg_id, count);
    }
}