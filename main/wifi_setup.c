#include "inc/wifi_setup.h"

static const char *TAG = "wifi_setup";

static EventGroupHandle_t s_wifi_event_group;
esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;
esp_netif_t *wifiAP;
esp_netif_t *wifiSTA;
static int s_retry_num = 0;
int wifi_connect_status = 0;

static void example_set_static_ip(esp_netif_t *netif)
{
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }

    if (esp_netif_set_ip_info(netif, &machine_info.wifi_info.ipInfo_STA) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
   // ESP_LOGD(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", EXAMPLE_STATIC_IP_ADDR, EXAMPLE_STATIC_NETMASK_ADDR, EXAMPLE_STATIC_GW_ADDR);
   // ESP_ERROR_CHECK(example_set_dns_server(netif, ipaddr_addr(EXAMPLE_MAIN_DNS_SERVER), ESP_NETIF_DNS_MAIN));
   // ESP_ERROR_CHECK(example_set_dns_server(netif, ipaddr_addr(EXAMPLE_BACKUP_DNS_SERVER), ESP_NETIF_DNS_BACKUP));
}





static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Initiating AP connection...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        example_set_static_ip(arg);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < machine_info.wifi_info.max_retries)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retrying to connect to the AP, try %d...",s_retry_num);
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGE(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
void from_sta_to_ap(void)
{
    ESP_LOGI(TAG, "Entering from_STA_to_AP");

    if (wifi_connect_status == 1)
    {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    }

    esp_netif_dhcps_stop(wifiAP);
    esp_netif_set_ip_info(wifiAP, &machine_info.wifi_info.ipInfo_AP);
    esp_netif_dhcps_start(wifiAP);
    wifi_config_t wifi_config = {0};
    wifi_config.ap.max_connection = 1;
    strncpy((char *)wifi_config.ap.ssid, machine_info.wifi_info.AP_ssid, machine_info.wifi_info.AP_ssid_len);
    strncpy((char *)wifi_config.ap.password, machine_info.wifi_info.AP_password, machine_info.wifi_info.AP_password_len);
    if (machine_info.wifi_info.AP_password_len == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    else
    {
        wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID: %s password: %s", machine_info.wifi_info.AP_ssid, machine_info.wifi_info.AP_password);
    wifi_connect_status = 0;
}

void from_ap_to_sta(void)
{
    ESP_LOGI(TAG, "Entering from_AP_to_STA");
    if (wifi_connect_status == 0)
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    }
    //esp_netif_dhcps_stop(wifiSTA);
    //esp_netif_set_ip_info(wifiSTA, &machine_info.wifi_info.ipInfo_STA);
    //esp_netif_dhcps_start(wifiSTA);
    s_wifi_event_group = xEventGroupCreate();
    wifi_config_t wifi_config = {0};
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    strncpy((char *)wifi_config.sta.ssid, machine_info.wifi_info.STA_ssid, machine_info.wifi_info.STA_ssid_len);
    //strncpy((char *)wifi_config.sta.ssid, "Jemny", machine_info.wifi_info.STA_ssid_len);
    strncpy((char *)wifi_config.sta.password, machine_info.wifi_info.STA_password, machine_info.wifi_info.STA_password_len);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to AP SSID: %s password: %s", machine_info.wifi_info.STA_ssid, machine_info.wifi_info.STA_password);
        wifi_connect_status = 1;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect AP to SSID: %s, password: %s", machine_info.wifi_info.STA_ssid, machine_info.wifi_info.STA_password);
            wifi_connect_status = 0;
            from_sta_to_ap();
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
            wifi_connect_status = 0;
    }
    vEventGroupDelete(s_wifi_event_group);
}

void wifi_init()
{
    ESP_LOGI(TAG, "wifi_init begin");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifiAP = esp_netif_create_default_wifi_ap();
    wifiSTA = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, wifiSTA, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, wifiSTA, &instance_got_ip));
    setup_server();
}

void wifi_task(void *pvParameters)
{
    wifi_init();
    from_ap_to_sta();
    // from_sta_to_ap();
    //wifi_init_02();
    while (1)
    {  
        if (wifi_connect_status)
        {
            //mqtt_app_start();
            vTaskDelete(NULL);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
