/**
 * @file Config.cpp
 * @brief Реализация управления конфигурацией системы
 */

#include "Config/Config.hpp"
#include <SD_MMC.h>
#include <ArduinoJson.h>

// Внешние объявления
extern bool sd_initialized;
extern Settings settings;

/**
 * @brief Загрузка настроек с SD карты
 */
void loadSettings()
{
    if (!sd_initialized)
    {
        Serial.println("No SD card, using default settings");
        return;
    }

    File file = SD_MMC.open("/settings.json");
    if (!file)
    {
        Serial.println("No settings file found, using defaults");
        return;
    }

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (!error)
    {
        settings.distance = doc["distance"] | 380;
        settings.interval = doc["interval"] | 5000;
        settings.threshold = doc["threshold"] | 160;
        settings.area = doc["area"] | 500;
        settings.dark_min = doc["dark_min"] | 0.2;
        settings.dark_max = doc["dark_max"] | 0.8;
        settings.texture = doc["texture"] | 500.0;
        settings.max_files = doc["max_files"] | 250;
        settings.roi_width = doc["roi_width"] | 80;
        settings.roi_height = doc["roi_height"] | 60;
        settings.roi_x = doc["roi_x"] | 40;
        settings.roi_y = doc["roi_y"] | 30;
        settings.ap_ssid = doc["ap_ssid"] | "CarDetector";
        settings.ap_password = doc["ap_password"] | "12345678";

        Serial.println("Settings loaded from SD card");
    }
    else
    {
        Serial.println("Error loading settings, using defaults");
    }
}

/**
 * @brief Сохранение настроек на SD карту
 */
void saveSettings()
{
    if (!sd_initialized)
        return;

    File file = SD_MMC.open("/settings.json", FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open settings file for writing");
        return;
    }

    DynamicJsonDocument doc(2048);
    doc["distance"] = settings.distance;
    doc["interval"] = settings.interval;
    doc["threshold"] = settings.threshold;
    doc["area"] = settings.area;
    doc["dark_min"] = settings.dark_min;
    doc["dark_max"] = settings.dark_max;
    doc["texture"] = settings.texture;
    doc["max_files"] = settings.max_files;
    doc["roi_width"] = settings.roi_width;
    doc["roi_height"] = settings.roi_height;
    doc["roi_x"] = settings.roi_x;
    doc["roi_y"] = settings.roi_y;
    doc["ap_ssid"] = settings.ap_ssid;
    doc["ap_password"] = settings.ap_password;

    if (serializeJson(doc, file) == 0)
    {
        Serial.println("Failed to write settings");
    }
    else
    {
        Serial.println("Settings saved successfully");
    }
    file.close();
}

/**
 * @brief Обновление координат ROI
 */
void updateROICoordinates()
{
    if (settings.roi_x == 0 && settings.roi_y == 0)
    {
        settings.roi_x = (160 - settings.roi_width) / 2;
        settings.roi_y = (120 - settings.roi_height) / 2;
    }

    settings.roi_x = max(0, min(settings.roi_x, 160 - settings.roi_width));
    settings.roi_y = max(0, min(settings.roi_y, 120 - settings.roi_height));

    Serial.printf("ROI configured: x=%d, y=%d, width=%d, height=%d\n", 
                  settings.roi_x, settings.roi_y, settings.roi_width, settings.roi_height);
}