// #ifndef __MAIN_FILE__
// #define __MAIN_FILE__

// #include "Arduino.h"

// #include "esp_camera.h"
// #include "FS.h"
// #include "SD_MMC.h"
// #include <WiFi.h>
// #include <WebServer.h>
// #include <ArduinoJson.h>
// #include <Preferences.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>


// #define FLASH_GPIO_NUM 4

// #define PWDN_GPIO_NUM 32
// #define RESET_GPIO_NUM -1
// #define XCLK_GPIO_NUM 0
// #define SIOD_GPIO_NUM 26
// #define SIOC_GPIO_NUM 27

// #define Y9_GPIO_NUM 35
// #define Y8_GPIO_NUM 34
// #define Y7_GPIO_NUM 39
// #define Y6_GPIO_NUM 36
// #define Y5_GPIO_NUM 21
// #define Y4_GPIO_NUM 19
// #define Y3_GPIO_NUM 18
// #define Y2_GPIO_NUM 5
// #define VSYNC_GPIO_NUM 25
// #define HREF_GPIO_NUM 23
// #define PCLK_GPIO_NUM 22


// uint16_t measure(void);

// void carDetection(void);
// void setupPreferences(void);
// void onFlash(void);
// void offFlash(void);
// void reverseFlash(void);
// void setupFlash(void);
// void setupWebServer(void);
// void setupCamera(void);
// void setupSDCard(void);
// void loadPreferences(void);
// void savePreferences(void);
// void loadSettings(void);
// void saveSettings(void);
// void updateROICoordinates(void);

// bool savePhotoToSD(const char *filename, camera_fb_t *fb, const DynamicJsonDocument &doc);

// String getMainPage(void);
// String getDetectionSettingsPage(void);
// String getWifiSettingsPage(void);
// String getROISettingsPage(void);

// #endif



/**
 * @file Main.hpp
 * @brief Главный заголовочный файл системы детектирования автомобилей
 */

#ifndef MAIN_HPP
#define MAIN_HPP

#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include "Config/Config.hpp"


// Пин вспышки
#define FLASH_GPIO_NUM     4


// Внешние объявления глобальных объектов
extern Preferences preferences;
extern WebServer server;
extern Settings settings;

// Внешние объявления глобальных переменных состояния
extern int lastDistance;
extern int resDistance;
extern int photoNumber;
extern bool sd_initialized;
extern bool camera_initialized;
extern bool car_detected;
extern unsigned long timeInterval;
extern unsigned long timeblink;

// Прототипы функций
void setup();
void loop();

#endif // MAIN_HPP