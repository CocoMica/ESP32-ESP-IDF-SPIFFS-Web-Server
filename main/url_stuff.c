#include "url_stuff.h"

int led_state = 0;
char index_html[4096];
char index_JS[4096];
char styles_CSS[4096];
char response_data[4096];
static const char *TAG = "URL stuff"; // TAG for debug

void initi_web_page_buffer(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    memset((void *)index_html, 0, sizeof(index_html));
    memset((void *)index_JS, 0, sizeof(index_JS));
    memset((void *)styles_CSS, 0, sizeof(styles_CSS));
    struct stat st;
    struct stat st2;
    struct stat st3;
    if (stat(INDEX_HTML_PATH, &st))
    {
        ESP_LOGE(TAG, "index.html not found");
        return;
    }

    if (stat(INDEX_JS_PATH, &st2))
    {
        ESP_LOGE(TAG, "script.js not found");
        return;
    }
    if (stat(STYLES_CSS_PATH, &st3))
    {
        ESP_LOGE(TAG, "styles.css not found");
        return;
    }
    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (fread(index_html, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);

    FILE *fp2 = fopen(INDEX_JS_PATH, "r");
    if (fread(index_JS, st2.st_size, 1, fp2) == 0)
    {
        ESP_LOGE(TAG, "JS fread failed");
    }
    fclose(fp2);

    FILE *fp3 = fopen(STYLES_CSS_PATH, "r");
    if (fread(styles_CSS, st3.st_size, 1, fp3) == 0)
    {
        ESP_LOGE(TAG, "CSS fread failed");
    }
    fclose(fp3);
}
esp_err_t script_js_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, index_JS, strlen(index_JS));
    return ESP_OK;
}
esp_err_t styles_css_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, styles_CSS, strlen(styles_CSS));
    return ESP_OK;
}

esp_err_t send_web_page(httpd_req_t *req)
{
    int response;
    if (led_state)
    {
        sprintf(response_data, index_html, "ON");
    }
    else
    {
        sprintf(response_data, index_html, "OFF");
    }
    response = httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);
    return response;
}
esp_err_t get_req_handler(httpd_req_t *req)
{
    return send_web_page(req);
}

esp_err_t led_on_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 1);
    led_state = 1;
    return send_web_page(req);
}

esp_err_t led_off_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 0);
    led_state = 0;
    return send_web_page(req);
}

httpd_uri_t uri_script_js = {
    .uri = "/script.js",
    .method = HTTP_GET,
    .handler = script_js_handler,
    .user_ctx = NULL};

httpd_uri_t uri_styles_css = {
    .uri = "/styles.css",
    .method = HTTP_GET,
    .handler = styles_css_handler,
    .user_ctx = NULL};

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_req_handler,
    .user_ctx = NULL};

httpd_uri_t uri_on = {
    .uri = "/led2on",
    .method = HTTP_GET,
    .handler = led_on_handler,
    .user_ctx = NULL};

httpd_uri_t uri_off = {
    .uri = "/led2off",
    .method = HTTP_GET,
    .handler = led_off_handler,
    .user_ctx = NULL};

httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_on);
        httpd_register_uri_handler(server, &uri_off);
        httpd_register_uri_handler(server, &uri_script_js);
        httpd_register_uri_handler(server, &uri_styles_css);
    }

    return server;
}