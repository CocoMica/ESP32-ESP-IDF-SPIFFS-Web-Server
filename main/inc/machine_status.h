#ifndef MACHINE_STATUS_H
#define MACHINE_STATUS_H
#include "esp_system.h"
#include "esp_log.h"
#include "storage.h"


void ESP_reset_reason();
void error_handler();

#endif