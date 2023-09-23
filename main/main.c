#include "inc/main.h"
static const char *TAG = "MAIN"; // TAG for debug
Machine_Info_t machine_info = {0};

void app_main()
{
    machine_setup_routine();

    xTaskCreate(read_ws_msg, "read_ws_msg", 4096, NULL, 1, NULL);
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 1, NULL);
}

void machine_setup_routine()
{
    ESP_LOGI(TAG, "Main app starting\n");
    // Initialize NVS
    esp_err_t result = nvs_flash_init();
#ifdef CLEAN_NVS_FIRST
        ESP_LOGW(TAG, "Erasing NVS Flash first");
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
#else
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
#endif
    if (result != ESP_OK)
    {
        error_handler();
    }
    // load general information from NVS
    result = nvs_get_general_information(true);
    if (result != ESP_OK)
    {
        error_handler();
    }
    // check for ESP reset reason
    if (machine_info.general_info.last_reset_reason != esp_reset_reason())
    {
        machine_info.general_info.last_reset_reason = esp_reset_reason();
        nvs_set_general_information(machine_info.general_info);
    }
    ESP_reset_reason();
    // load wifi information from NVs
    result = nvs_get_wifi_information(true);
    if (result != ESP_OK)
    {
        error_handler();
    }
    // load web pages from SPIFFS
    result = load_spiffs_pages();
    if (result != ESP_OK)
    {
        error_handler();
    }

    // gpio setup
    gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}
