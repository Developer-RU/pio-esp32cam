/**
 * @file WebServerManager.hpp
 * @brief Управление веб-сервером и обработка запросов
 */

#ifndef WEBSERVER_MANAGER_HPP
#define WEBSERVER_MANAGER_HPP

#include <WebServer.h>

// Прототипы функций
void setupWebServer();
void handleRoot();
void handleDetectionSettings();
void handleWifiSettings();
void handleROISettings();
void handleSaveDetection();
void handleSaveWifi();
void handleSaveROI();
void handleListPhotos();
void handleDeletePhoto();

#endif // WEBSERVER_MANAGER_HPP