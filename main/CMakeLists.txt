set(srcs "main.c" "url_stuff.c" "wifi_setup.c" "received_messages.c" "firmware_update.c" "mqtt_stuff.c" "ota_stuff.c" "machine_status.c" "storage.c")
idf_component_register(SRCS ${srcs} INCLUDE_DIRS ".")
target_add_binary_data(${COMPONENT_TARGET} "mqtt_eclipseprojects_io.pem" TEXT)

spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)

