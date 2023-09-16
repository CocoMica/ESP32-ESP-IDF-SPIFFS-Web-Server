#include "inc/received_messages.h"
QueueHandle_t wsReceivedQueue;

static const char *TAG = "received_messages";
void init_queues(void){
    wsReceivedQueue = xQueueCreate(10, MAX_STRING_LENGTH*sizeof(char));
    if(wsReceivedQueue == NULL){
        ESP_LOGE(TAG, "failed to create wsReceiveQueue\n");
    }
}

void sendStringToQueue(const char *str) {
    if (xQueueSend(wsReceivedQueue, str, portMAX_DELAY) != pdPASS) {
        ESP_LOGE(TAG, "failed to write to wsReceiveQueue\n");
    }
}

void receiveStringFromQueue() {
    char receivedString[MAX_STRING_LENGTH];
    if (xQueueReceive(wsReceivedQueue, receivedString, portMAX_DELAY) == pdPASS) {
        ESP_LOGI(TAG, "%s\n", receivedString);
    } else {
        ESP_LOGE(TAG, "failed to read from wsReceiveQueue\n");
    }
}

char* getSendingCommand(const char *str){
    sendStringToQueue(str);
    int size = 10;
    if(strcmp(str, "info from browser: 2") == 0){
        size = 4;
        char* returnMsg = (char*)malloc(size*sizeof(char));
        if(returnMsg == NULL){
            return "";
        }
        strcpy(returnMsg, "HI!");
        return returnMsg;
    }
    char* returnMsg = (char*)malloc(size*sizeof(char));
    if(returnMsg == NULL){
            return "";
    }
    strcpy(returnMsg, "123456789");
    return returnMsg;

}

void read_ws_msg(void *pvParameters){
    init_queues();
    while(1){
        receiveStringFromQueue();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}