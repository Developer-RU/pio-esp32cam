/**
 * @file HtmlPages.hpp
 * @brief Генерация HTML страниц
 */

#ifndef HTML_PAGES_HPP
#define HTML_PAGES_HPP

#include <Arduino.h>
#include <SD_MMC.h>

// Прототипы функций
String getBaseTemplate(const String &title, const String &content);
String getMainPage();
String getDetectionSettingsPage();
String getWifiSettingsPage();
String getROISettingsPage();
String getPhotosListPage();

#endif // HTML_PAGES_HPP