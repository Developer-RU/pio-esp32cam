#ifndef __MAIN_FILE__
#define __MAIN_FILE__

#include "Arduino.h"

#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Global.hpp"

#define FLASH_GPIO_NUM 4

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22


uint16_t measure(void);

void carDetection(void);
void setupPreferences(void);
void onFlash(void);
void offFlash(void);
void reverseFlash(void);
void setupFlash(void);
void setupWebServer(void);
void setupCamera(void);
void setupSDCard(void);
void loadPreferences(void);
void savePreferences(void);
void loadSettings(void);
void saveSettings(void);
void updateROICoordinates(void);

bool savePhotoToSD(const char *filename, camera_fb_t *fb, const DynamicJsonDocument &doc);

String getMainPage(void);
String getDetectionSettingsPage(void);
String getWifiSettingsPage(void);
String getROISettingsPage(void);

void DistanceMeterTask(void *pvParameters);

#endif