#include "inc/web_server.h"

static const char *TAG = "web server"; // TAG for debug
httpd_uri_t uri_get_1;
httpd_uri_t uri_post;
httpd_uri_t uri_post_AP_Info;

/* URI handler structure for GET /uri */
httpd_uri_t uri_get_1 = {
    .uri      = "/uri",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post = {
    .uri      = "/submit",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_post_AP_Info = {
    .uri      = "/submit_AP_Info",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};



/* Our URI handler function to be called during GET /uri request */
esp_err_t get_handler(httpd_req_t *req)
{
    /* Send a simple response */
    httpd_resp_send(req, NVS_Submit_HTML, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* Our URI handler function to be called during POST /uri request */
esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }
    // Initialize variables to store the extracted values
    char SSID[32] = "";
    char Password[64] = "";

    // Tokenize the string using '&' as the delimiter
    char *token = strtok(content, "&");
   // Iterate through the tokens
    while (token != NULL) {
        // Check if the token starts with "SSID="
        if (strncmp(token, "SSID=", 5) == 0) {
            strcpy(SSID, token + 5); // Copy the value after "SSID=" into SSID
        }
        // Check if the token starts with "Password="
        else if (strncmp(token, "Password=", 9) == 0) {
            strcpy(Password, token + 9); // Copy the value after "Password=" into Password
        }
        token = strtok(NULL, "&"); // Get the next token
    }

    // Output the extracted values
    printf("SSID: %s\n", SSID);
    printf("Password: %s\n", Password);

    //as a temp test, setting the AP credentials to the new data:
    strcpy(machine_info.wifi_info.AP_ssid, SSID);
    strcpy(machine_info.wifi_info.AP_password, Password);
    nvs_set_wifi_information(machine_info.wifi_info);

    ESP_LOGE(TAG, "response: %s", content);
    /* Send a simple response */
    //const char resp[] = "URI POST Response";
    //httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


