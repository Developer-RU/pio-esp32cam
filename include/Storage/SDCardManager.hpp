/**
 * @file SDCardManager.hpp
 * @brief Управление SD картой и файловой системой
 */

#ifndef SDCARD_MANAGER_HPP
#define SDCARD_MANAGER_HPP

#include <Arduino.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>
#include <esp_camera.h>

// Прототипы функций
bool setupSDCard();
bool savePhotoToSD(const char *filename, camera_fb_t *fb, const DynamicJsonDocument &doc);
bool verifyFile(const String &path, size_t expectedSize);
String listFiles();

#endif // SDCARD_MANAGER_HPP