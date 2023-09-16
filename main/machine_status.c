#include "inc/machine_status.h"
static const char *TAG = "machine_status";

void ESP_reset_reason()
{
    switch (machine_info.general_info.last_reset_reason)
    {
    case ESP_RST_POWERON:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Power-on reset");
        break;
    case ESP_RST_EXT:
        ESP_LOGW(TAG, "ESP32 last Reset reason: External reset");
        break;
    case ESP_RST_SW:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Software reset");
        break;
    case ESP_RST_PANIC:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Panic reset");
        break;
    case ESP_RST_INT_WDT:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Internal watchdog timer reset");
        break;
    case ESP_RST_TASK_WDT:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Task watchdog timer reset");
        break;
    case ESP_RST_WDT:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Other watchdog reset");
        break;
    case ESP_RST_DEEPSLEEP:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Deep sleep reset");
        break;
    case ESP_RST_BROWNOUT:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Brownout reset");
        break;
    case ESP_RST_SDIO:
        ESP_LOGW(TAG, "ESP32 last Reset reason: SDIO reset");
        break;
    default:
        ESP_LOGW(TAG, "ESP32 last Reset reason: Unknown");
        break;
    }
}

void error_handler()
{
    ESP_LOGE(TAG, "Critial failure occured. ESP restarting in 5 seconds...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    esp_restart();
}