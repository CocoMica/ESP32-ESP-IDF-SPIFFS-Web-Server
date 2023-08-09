# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Softwares/ESP-IDF/esp-idf/components/bootloader/subproject"
  "D:/Hobby_Coding/spiffs_seb_server/ESP32-ESP-IDF-SPIFFS-Web-Server/build/bootloader"
  "D:/Hobby_Coding/spiffs_seb_server/ESP32-ESP-IDF-SPIFFS-Web-Server/build/bootloader-prefix"
  "D:/Hobby_Coding/spiffs_seb_server/ESP32-ESP-IDF-SPIFFS-Web-Server/build/bootloader-prefix/tmp"
  "D:/Hobby_Coding/spiffs_seb_server/ESP32-ESP-IDF-SPIFFS-Web-Server/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Hobby_Coding/spiffs_seb_server/ESP32-ESP-IDF-SPIFFS-Web-Server/build/bootloader-prefix/src"
  "D:/Hobby_Coding/spiffs_seb_server/ESP32-ESP-IDF-SPIFFS-Web-Server/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Hobby_Coding/spiffs_seb_server/ESP32-ESP-IDF-SPIFFS-Web-Server/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Hobby_Coding/spiffs_seb_server/ESP32-ESP-IDF-SPIFFS-Web-Server/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
