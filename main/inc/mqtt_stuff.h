#ifndef MQTT_STUFF_H
#define MQTT_STUFF_H
#include "mqtt_client.h"

#define CONFIG_BROKER_BIN_SIZE_TO_SEND 20000
#define CONFIG_BROKER_URI "mqtts://bbcbdbaf.ala.us-east-1.emqxsl.com:8883"
extern const uint8_t mqtt_eclipseprojects_io_pem_start[] asm("_binary_mqtt_eclipseprojects_io_pem_start");
extern const uint8_t mqtt_eclipseprojects_io_pem_end[] asm("_binary_mqtt_eclipseprojects_io_pem_end");

void mqtt_app_start(void);
int startsWith(const char *topic, const char *prefix);

#endif