set(srcs "main.c" "connect_wifi.c" "url_stuff.c" "wifi_setup.c")
idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)
