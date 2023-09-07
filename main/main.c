#include "inc/main.h"

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "Main app starting\n");
    xTaskCreate(read_ws_msg, "read_ws_msg", 4096, NULL, 1, NULL);
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 1, NULL);
    int count = 0;
    send_message = "gg";
    // char sendMsg[MAX_STRING_LENGTH];
    while (1)
    {
        if (wifi_connect_status)
        {
            mqtt_app_start();
            break;
            count++;
            // ESP_LOGI(TAG, "Sending msg: %d\n", count);
            // snprintf(sendMsg, sizeof(sendMsg), "this is from main loop: %d", count);
            // esp_err_t ret =  trigger_async_send("Hello!");
            // ESP_LOGI(TAG, "Sending msg %d success: %s\n", count, esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
    ESP_LOGI(TAG, "exiting while loop");
}
