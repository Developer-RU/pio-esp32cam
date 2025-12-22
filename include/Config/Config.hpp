/**
 * @file Config.hpp
 * @brief Конфигурация системы и управление настройками
 */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Структура настроек системы
 */
struct Settings
{
    int distance = 380;
    int interval = 5000;
    int threshold = 160;
    int area = 50;
    float dark_min = 0.2;
    float dark_max = 0.8;
    float texture = 500.0;
    int max_files = 250;
    int roi_width = 80;
    int roi_height = 60;
    int roi_x = 40;
    int roi_y = 30;
    String ap_ssid = "CarDetector";
    String ap_password = "12345678";
};

// Внешнее объявление глобальной структуры настроек
extern Settings settings;

// Функции управления настройками
void loadSettings();
void saveSettings();
void updateROICoordinates();

#endif // CONFIG_HPP