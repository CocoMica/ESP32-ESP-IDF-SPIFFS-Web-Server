#include "inc/url_stuff.h"


int led_state = 0;
char index_html[4096];
char index_JS[4096];
char styles_CSS[4096];
char response_data[4096];
char image[25000];
static const char *TAG = "URL stuff"; // TAG for debug
char ws_sendMsg[MAX_STRING_LENGTH] = "";
char *send_message;
//new stuff--------------------------------------------------------------------------------
httpd_handle_t server = NULL;
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};
static void ws_async_send(void *arg)
{
    httpd_ws_frame_t ws_pkt;
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;    
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)ws_sendMsg;
    ws_pkt.len = strlen(ws_sendMsg);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.final = true;
    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}
static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req, char *sendMsg)
{
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    strcpy(ws_sendMsg, sendMsg);//local msg
    //strcpy(ws_sendMsg, send_message);//global msg
   return httpd_queue_work(handle, ws_async_send, resp_arg);
}
static esp_err_t handle_ws_req(httpd_req_t *req)
{     
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    char* sendMsg = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len)
    {
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL)
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        sendMsg = getSendingCommand((char *)ws_pkt.payload);
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT)
    {
        free(buf);
        return trigger_async_send(req->handle, req, sendMsg);//-------------------------------------------------------------------------
        free(sendMsg);
    }
    return ESP_OK;
}

httpd_uri_t ws = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = handle_ws_req,
    .user_ctx = NULL,
    .is_websocket = true};

//end of new function----------------------------------------------------------------

void initi_web_page_buffer(void)
{
    //init config to read from spiffs
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    //get index.html
    memset((void *)index_html, 0, sizeof(index_html));
    struct stat st;
    if (stat(INDEX_HTML_PATH, &st))
    {
        ESP_LOGE(TAG, "index.html not found");
        return;
    }
    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (fread(index_html, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);

    //get script.js
    struct stat st2;
    memset((void *)index_JS, 0, sizeof(index_JS));
    if (stat(INDEX_JS_PATH, &st2))
    {
        ESP_LOGE(TAG, "script.js not found");
        return;
    }
    FILE *fp2 = fopen(INDEX_JS_PATH, "r");
    if (fread(index_JS, st2.st_size, 1, fp2) == 0)
    {
        ESP_LOGE(TAG, "JS fread failed");
    }
    fclose(fp2);

    //get styles.css
    struct stat st3;
    memset((void *)styles_CSS, 0, sizeof(styles_CSS));
    if (stat(STYLES_CSS_PATH, &st3))
    {
        ESP_LOGE(TAG, "styles.css not found");
        return;
    }
    FILE *fp3 = fopen(STYLES_CSS_PATH, "r");
    if (fread(styles_CSS, st3.st_size, 1, fp3) == 0)
    {
        ESP_LOGE(TAG, "CSS fread failed");
    }
    fclose(fp3);

    //get the image
    struct stat st4;
    memset((void *)image, 0, sizeof(image));
    if (stat(IMAGE_PATH, &st4))
    {
        ESP_LOGE(TAG, "image not found");
        return;
    }

    FILE *fp4 = fopen(IMAGE_PATH, "r");
    if (fread(image, st4.st_size, 1, fp4) == 0)
    {
        ESP_LOGE(TAG, "image fread failed");
    }
    fclose(fp4);

    esp_vfs_spiffs_unregister(conf.partition_label);
}

//---index.html functions---//
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
//get request handler
esp_err_t get_req_handler(httpd_req_t *req)
{
    return send_web_page(req);
}
httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_req_handler,
    .user_ctx = NULL};

//LED on handler
esp_err_t led_on_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 1);
    led_state = 1;
    return send_web_page(req);
}
httpd_uri_t uri_on = {
    .uri = "/led2on",
    .method = HTTP_GET,
    .handler = led_on_handler,
    .user_ctx = NULL};

//LED off handler
esp_err_t led_off_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 0);
    led_state = 0;
    return send_web_page(req);
}
httpd_uri_t uri_off = {
    .uri = "/led2off",
    .method = HTTP_GET,
    .handler = led_off_handler,
    .user_ctx = NULL};
//---end of index.html functions---//

//script.js functions
httpd_uri_t uri_script_js = {
    .uri = "/script.js",
    .method = HTTP_GET,
    .handler = script_js_handler,
    .user_ctx = NULL};
esp_err_t script_js_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, index_JS, strlen(index_JS));
    return ESP_OK;
}

//styles.css functions
esp_err_t styles_css_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, styles_CSS, strlen(styles_CSS));
    return ESP_OK;
}
httpd_uri_t uri_styles_css = {
    .uri = "/styles.css",
    .method = HTTP_GET,
    .handler = styles_css_handler,
    .user_ctx = NULL};


//image functions
esp_err_t image_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, image, strlen(image));
    return ESP_OK;
}
httpd_uri_t uri_image = {
    .uri = "/test.png",
    .method = HTTP_GET,
    .handler = image_handler,
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
        //httpd_register_uri_handler(server, &uri_image);
        httpd_register_uri_handler(server, &ws);
    }

    return server;
}