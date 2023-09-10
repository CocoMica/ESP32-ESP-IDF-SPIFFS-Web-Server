#include "inc/wifi_setup.h"
static const char *TAG = "wifi_setup";
static EventGroupHandle_t s_wifi_event_group;
esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;
esp_netif_t *wifiAP;
static int s_retry_num = 0;
int wifi_connect_status = 0;
Wifi_Info_t nvs_wifi_info = {0};

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < nvs_wifi_info.max_retries)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        wifi_connect_status = 0;
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_connect_status = 1;
    }
}
void from_sta_to_ap(void)
{
    ESP_LOGI(TAG, "Entering from_sta_to_ap.\n");
    esp_netif_ip_info_t ipInfo;
    if (wifi_connect_status == 1)
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    }
    IP4_ADDR(&ipInfo.ip, nvs_wifi_info.AP_IP[0], nvs_wifi_info.AP_IP[1], nvs_wifi_info.AP_IP[2], nvs_wifi_info.AP_IP[3]);
    IP4_ADDR(&ipInfo.gw, nvs_wifi_info.AP_GW[0], nvs_wifi_info.AP_GW[1], nvs_wifi_info.AP_GW[2], nvs_wifi_info.AP_GW[3]);
    IP4_ADDR(&ipInfo.netmask, nvs_wifi_info.AP_netmask[0], nvs_wifi_info.AP_netmask[1], nvs_wifi_info.AP_netmask[2], nvs_wifi_info.AP_netmask[3]);
    esp_netif_dhcps_stop(wifiAP);
    esp_netif_set_ip_info(wifiAP, &ipInfo);
    esp_netif_dhcps_start(wifiAP);
    ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&ipInfo.ip));
    ESP_LOGI(TAG, "GW: " IPSTR, IP2STR(&ipInfo.gw));
    ESP_LOGI(TAG, "Mask: " IPSTR, IP2STR(&ipInfo.netmask));
    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    strncpy((char *)wifi_config.ap.ssid, nvs_wifi_info.AP_ssid, nvs_wifi_info.AP_ssid_len);
    strncpy((char *)wifi_config.ap.password, nvs_wifi_info.AP_password, nvs_wifi_info.AP_password_len);
    if (nvs_wifi_info.AP_password_len == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID: %s password: %s", nvs_wifi_info.AP_ssid, nvs_wifi_info.AP_password);
    wifi_connect_status = 0;
}

void from_ap_to_sta(void)
{
    ESP_LOGI(TAG, "Entering from_ap_to_sta.\n");
    if (wifi_connect_status == 0)
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    }
    s_wifi_event_group = xEventGroupCreate();
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    strncpy((char *)wifi_config.sta.ssid, nvs_wifi_info.STA_ssid, nvs_wifi_info.STA_ssid_len);
    strncpy((char *)wifi_config.sta.password, nvs_wifi_info.STA_password, nvs_wifi_info.STA_password_len);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE,
                                           pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID: %s password: %s", nvs_wifi_info.STA_ssid, nvs_wifi_info.STA_password);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID: %s, password: %s", nvs_wifi_info.STA_ssid, nvs_wifi_info.STA_password);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    vEventGroupDelete(s_wifi_event_group);
    wifi_connect_status = 1;
}

void wifi_init()
{

    ESP_LOGI(TAG, "wifi_init begin\n");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifiAP = esp_netif_create_default_wifi_ap();
    nvs_wifi_info = nvs_get_wifi_information(false);

    esp_netif_create_default_wifi_sta();
    initi_web_page_buffer();
    setup_server();

}

void wifi_task(void *pvParameters)
{
    wifi_init();
    from_ap_to_sta();
    //from_sta_to_ap();
    while (1)
    {
        //ESP_LOGE(TAG, "Configuring as a STA\n");
        //from_ap_to_sta();
        vTaskDelay(pdMS_TO_TICKS(60000));
        //ESP_LOGE(TAG, "Configuring as a AP\n");
        //from_sta_to_ap();
        //vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
