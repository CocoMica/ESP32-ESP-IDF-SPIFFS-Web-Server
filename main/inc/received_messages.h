#ifndef RECEIVED_MESSAGES_H
#define RECEIVED_MESSAGES_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "spi_flash_mmap.h"
#include <esp_http_server.h>
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "url_stuff.h"

#define MAX_STRING_LENGTH 50
extern QueueHandle_t wsReceivedQueue;


void init_queues(void);
void receiveStringFromQueue();
void sendStringToQueue(const char *str);
void read_ws_msg(void *pvParameters);
char* getSendingCommand(const char *str);



#endif