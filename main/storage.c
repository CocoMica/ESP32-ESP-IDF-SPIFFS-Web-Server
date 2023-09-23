#include "inc/storage.h"
static const char *TAG = "storage";

char NVS_Submit_HTML[4096];
char NVS_Index_HTML[4096];
char NVS_Index_JS[4096];
char NVS_Styles_CSS[4096];
char response_data[4096];
char NVS_Image_Test[25000];

esp_err_t load_spiffs_pages()
{
    // init config to read from spiffs
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    // get index.html
    memset((void *)NVS_Index_HTML, 0, sizeof(NVS_Index_HTML));
    struct stat st;
    if (stat(INDEX_HTML_PATH, &st))
    {
        ESP_LOGE(TAG, "index.html not found");
        goto exit_safe;
    }
    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (fread(NVS_Index_HTML, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
        goto exit_safe;
    }
    fclose(fp);

    // get script.js
    struct stat st2;
    memset((void *)NVS_Index_JS, 0, sizeof(NVS_Index_JS));
    if (stat(INDEX_JS_PATH, &st2))
    {
        ESP_LOGE(TAG, "script.js not found");
        goto exit_safe;
    }
    FILE *fp2 = fopen(INDEX_JS_PATH, "r");
    if (fread(NVS_Index_JS, st2.st_size, 1, fp2) == 0)
    {
        ESP_LOGE(TAG, "JS fread failed");
        goto exit_safe;
    }
    fclose(fp2);

    // get styles.css
    struct stat st3;
    memset((void *)NVS_Styles_CSS, 0, sizeof(NVS_Styles_CSS));
    if (stat(STYLES_CSS_PATH, &st3))
    {
        ESP_LOGE(TAG, "styles.css not found");
        goto exit_safe;
    }
    FILE *fp3 = fopen(STYLES_CSS_PATH, "r");
    if (fread(NVS_Styles_CSS, st3.st_size, 1, fp3) == 0)
    {
        ESP_LOGE(TAG, "CSS fread failed");
        goto exit_safe;
    }
    fclose(fp3);

    // get the image
    struct stat st4;
    memset((void *)NVS_Image_Test, 0, sizeof(NVS_Image_Test));
    if (stat(IMAGE_PATH, &st4))
    {
        ESP_LOGE(TAG, "image not found");
        goto exit_safe;
    }

    FILE *fp4 = fopen(IMAGE_PATH, "r");
    if (fread(NVS_Image_Test, st4.st_size, 1, fp4) == 0)
    {
        ESP_LOGE(TAG, "image fread failed");
        goto exit_safe;
    }
    fclose(fp4);

    // get submit.html
    struct stat st5;
    memset((void *)NVS_Submit_HTML, 0, sizeof(NVS_Submit_HTML));

    if (stat(SUBMIT_HTML_PATH, &st5))
    {
        ESP_LOGE(TAG, "submit.html not found");
        goto exit_safe;
    }
    FILE *fp5 = fopen(SUBMIT_HTML_PATH, "r");
    if (fread(NVS_Submit_HTML, st5.st_size, 1, fp5) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
        goto exit_safe;
    }
    fclose(fp5);

    esp_vfs_spiffs_unregister(conf.partition_label);
    return ESP_OK;
exit_safe:
    esp_vfs_spiffs_unregister(conf.partition_label);
    return ESP_FAIL;
}


esp_err_t nvs_set_general_information(General_Info_t new_general_info)
{
    nvs_handle_t nvs_handle;
    esp_err_t result = nvs_flash_init();
    if (result != ESP_OK)
    {
        goto exit_safe;
    }

    result = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open nvs storage (%s)", esp_err_to_name(result));
        goto exit_safe;
    }

    result = nvs_set_blob(nvs_handle, NVS_GENERAL_INFO, (const void *)&new_general_info, sizeof(new_general_info));
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set blob (%s)", esp_err_to_name(result));
        goto exit_safe;
    }
    result = nvs_commit(nvs_handle);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit updates (%s)", esp_err_to_name(result));
        goto exit_safe;
    }
    nvs_close(nvs_handle);
    return ESP_OK;

exit_safe:
    nvs_close(nvs_handle);
    return ESP_FAIL;
}

esp_err_t nvs_get_general_information(bool print_out_information)
{
    nvs_handle_t nvs_handle;
    esp_err_t result = nvs_flash_init();
    if (result != ESP_OK)
    {
        goto exit_safe;
    }
    result = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open nvs storage (%s)", esp_err_to_name(result));
        goto exit_safe;
    }
    // read data
    size_t required_size;
    result = nvs_get_blob(nvs_handle, NVS_GENERAL_INFO, NULL, &required_size);
    result = nvs_get_blob(nvs_handle, NVS_GENERAL_INFO, (void *)&machine_info.general_info, &required_size);
    if (required_size != sizeof(machine_info.general_info))
    {
        ESP_LOGE(TAG, "Error in size mismatch between the nvs_handle and the struct it is decoded to! Setting up inital configuration...");
        result = ESP_ERR_NVS_NOT_FOUND;
    }

    switch (result)
    {
    case ESP_OK:
        if (print_out_information)
        {
            printf("machine_info.general_info.last_reset_reason = %d\n", machine_info.general_info.last_reset_reason);
        }

        break;
    case ESP_ERR_NVS_NOT_FOUND:
        required_size = sizeof(machine_info.general_info);
        machine_info.general_info.last_reset_reason = 0;
        ESP_LOGI(TAG, "Setting up general default configuration in NVS ... ");
        nvs_set_general_information(machine_info.general_info);
        break;
    default:
        printf("Error (%s) reading!\n", esp_err_to_name(result));
    }
    nvs_close(nvs_handle);
    return ESP_OK;

exit_safe:
    nvs_close(nvs_handle);
    return ESP_FAIL;
}

esp_err_t nvs_set_wifi_information(Wifi_Info_t new_wifi_info)
{
    nvs_handle_t nvs_handle;
    esp_err_t result = nvs_flash_init();
    if (result != ESP_OK)
    {
        goto exit_safe;
    }

    result = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open nvs storage (%s)", esp_err_to_name(result));
        goto exit_safe;
    }

    result = nvs_set_blob(nvs_handle, NVS_WIFI_INFO, (const void *)&new_wifi_info, sizeof(new_wifi_info));
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set blob (%s)", esp_err_to_name(result));
        goto exit_safe;
    }
    result = nvs_commit(nvs_handle);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit updates (%s)", esp_err_to_name(result));
        goto exit_safe;
    }
    nvs_close(nvs_handle);
    return ESP_OK;

exit_safe:
    nvs_close(nvs_handle);
    return ESP_FAIL;
}

esp_err_t nvs_get_wifi_information(bool print_out_information)
{
    esp_err_t result = nvs_flash_init();
    if (result != ESP_OK)
    {
        goto exit_safe;
    }
    nvs_handle_t nvs_handle;
    result = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open nvs storage (%s)", esp_err_to_name(result));
        goto exit_safe;
    }
    // read data
    size_t required_size;
    result = nvs_get_blob(nvs_handle, NVS_WIFI_INFO, NULL, &required_size);
    result = nvs_get_blob(nvs_handle, NVS_WIFI_INFO, (void *)&machine_info.wifi_info, &required_size);
    if (required_size != sizeof(machine_info.wifi_info))
    {
        ESP_LOGE(TAG, "Error in size mismatch between the nvs_handle and the struct it is decoded to! Setting up inital configuration...");
        result = ESP_ERR_NVS_NOT_FOUND;
    }
    switch (result)
    {
    case ESP_OK:
        if (print_out_information)
        {
            printf("machine_info.wifi_info.STA_ssid = %s\n", machine_info.wifi_info.STA_ssid);
            printf("machine_info.wifi_info.STA_ssid_len = %d\n", machine_info.wifi_info.STA_ssid_len);
            printf("machine_info.wifi_info.STA_password = %s\n", machine_info.wifi_info.STA_password);
            printf("machine_info.wifi_info.STA_password_len = %d\n", machine_info.wifi_info.STA_password_len);

            printf("machine_info.wifi_info.AP_ssid = %s\n", machine_info.wifi_info.AP_ssid);
            printf("machine_info.wifi_info.AP_ssidlen = %d\n", machine_info.wifi_info.AP_ssid_len);
            printf("machine_info.wifi_info.AP_password = %s\n", machine_info.wifi_info.AP_password);
            printf("machine_info.wifi_info.AP_password_len = %d\n", machine_info.wifi_info.AP_password_len);
            ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&machine_info.wifi_info.ipInfo.ip));
            ESP_LOGI(TAG, "GW: " IPSTR, IP2STR(&machine_info.wifi_info.ipInfo.gw));
            ESP_LOGI(TAG, "Mask: " IPSTR, IP2STR(&machine_info.wifi_info.ipInfo.netmask));

            printf("machine_info.wifi_info.max_retries = %d\n", machine_info.wifi_info.max_retries);
        }

        break;
    case ESP_ERR_NVS_NOT_FOUND:
        required_size = sizeof(machine_info.wifi_info);
        strcpy(machine_info.wifi_info.STA_ssid, CONFIG_ESP_WIFI_SSID);
        strcpy(machine_info.wifi_info.STA_password, CONFIG_ESP_WIFI_PASSWORD);
        machine_info.wifi_info.STA_ssid_len = strlen(CONFIG_ESP_WIFI_SSID);
        machine_info.wifi_info.STA_password_len = strlen(CONFIG_ESP_WIFI_PASSWORD);

        strcpy(machine_info.wifi_info.AP_ssid, CONFIG_ESP_AP_SSID);
        strcpy(machine_info.wifi_info.AP_password, CONFIG_ESP_AP_PASSWORD);
        machine_info.wifi_info.AP_ssid_len = strlen(CONFIG_ESP_AP_SSID);
        machine_info.wifi_info.AP_password_len = strlen(CONFIG_ESP_AP_PASSWORD);

        machine_info.wifi_info.AP_IP[0] = 192;
        machine_info.wifi_info.AP_IP[1] = 168;
        machine_info.wifi_info.AP_IP[2] = 2;
        machine_info.wifi_info.AP_IP[3] = 1;

        machine_info.wifi_info.AP_GW[0] = 192;
        machine_info.wifi_info.AP_GW[1] = 168;
        machine_info.wifi_info.AP_GW[2] = 2;
        machine_info.wifi_info.AP_GW[3] = 1;

        machine_info.wifi_info.AP_netmask[0] = 255;
        machine_info.wifi_info.AP_netmask[1] = 255;
        machine_info.wifi_info.AP_netmask[2] = 255;
        machine_info.wifi_info.AP_netmask[3] = 0;

        IP4_ADDR(&machine_info.wifi_info.ipInfo.ip, machine_info.wifi_info.AP_IP[0], machine_info.wifi_info.AP_IP[1], machine_info.wifi_info.AP_IP[2], machine_info.wifi_info.AP_IP[3]);
        IP4_ADDR(&machine_info.wifi_info.ipInfo.gw, machine_info.wifi_info.AP_GW[0], machine_info.wifi_info.AP_GW[1], machine_info.wifi_info.AP_GW[2], machine_info.wifi_info.AP_GW[3]);
        IP4_ADDR(&machine_info.wifi_info.ipInfo.netmask, machine_info.wifi_info.AP_netmask[0], machine_info.wifi_info.AP_netmask[1], machine_info.wifi_info.AP_netmask[2], machine_info.wifi_info.AP_netmask[3]);

        machine_info.wifi_info.max_retries = 5; // CONFIG_ESP_MAXIMUM_RETRY;

        ESP_LOGI(TAG, "Setting up wifi default configuration in NVS ... ");
        nvs_set_wifi_information(machine_info.wifi_info);
        break;
    default:
        printf("Error (%s) reading!\n", esp_err_to_name(result));
    }
    nvs_close(nvs_handle);
    return ESP_OK;

exit_safe:
    return ESP_FAIL;
}

void init_storage()
{

    ESP_LOGI(TAG, "initializing storage");
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/storage",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_err_t result = esp_vfs_spiffs_register(&config);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(result));
        goto exit_safe;
    }
    size_t total = 0, used = 0;
    result = esp_spiffs_info(config.partition_label, &total, &used);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS info (%s)", esp_err_to_name(result));
        esp_vfs_spiffs_unregister(config.partition_label);
        goto exit_safe;
    }
    else
    {
        ESP_LOGI(TAG, "Partition size total: %d, used: %d", total, used);
    }
    // opening the temp file
    FILE *f = fopen(TEMP_FILE_PATH, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open the temp file for reading.");
        goto exit_safe;
    }

    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    ESP_LOGI(TAG, "file contents: %s", line);

    // Open another file for writing; create it if it doesn't exist
    FILE *file = fopen("/storage/myfile2.txt", "w+");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file2 for writing");
    }
    else
    {
        const char *data = "Hello, SPIFFS!";
        fprintf(file, "%s", data);
        fclose(file);
        ESP_LOGI(TAG, "new File written: %s", data);
    }
    // Read data from the file
    char buffer[32];
    file = fopen("/storage/myfile2.txt", "r");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
    }
    else
    {
        if (fgets(buffer, sizeof(buffer), file) != NULL)
        {
            ESP_LOGI(TAG, "File content: %s", buffer);
        }
        fclose(file);
    }

exit_safe:
    if (esp_spiffs_mounted(config.partition_label))
    {
        esp_vfs_spiffs_unregister(config.partition_label);
    }
    return;
}
