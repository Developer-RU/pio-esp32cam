/**
 * @file HtmlPages.hpp
 * @brief Генерация HTML страниц
 */

#ifndef HTML_PAGES_HPP
#define HTML_PAGES_HPP

#include <Arduino.h>

// Прототипы функций
String getMainPage();
String getDetectionSettingsPage();
String getWifiSettingsPage();
String getROISettingsPage();

#endif // HTML_PAGES_HPP