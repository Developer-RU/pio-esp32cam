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