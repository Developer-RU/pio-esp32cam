/**
 * @file WebServerManager.cpp
 * @brief Реализация веб-сервера
 */

#include "Web/WebServerManager.hpp"
#include "Web/HtmlPages.hpp"
#include "Config/Config.hpp"
#include "Storage/SDCardManager.hpp"

// Внешние объявления
extern WebServer server;
extern Settings settings;

/**
 * @brief Настройка веб-сервера
 */
void setupWebServer()
{
    server.on("/", HTTP_GET, handleRoot);
    server.on("/detection", HTTP_GET, handleDetectionSettings);
    server.on("/wifi", HTTP_GET, handleWifiSettings);
    server.on("/roi", HTTP_GET, handleROISettings);
    server.on("/save_detection", HTTP_POST, handleSaveDetection);
    server.on("/save_wifi", HTTP_POST, handleSaveWifi);
    server.on("/save_roi", HTTP_POST, handleSaveROI);
    server.on("/list_photos", HTTP_GET, handleListPhotos);
    server.on("/delete_photo", HTTP_POST, handleDeletePhoto);

    server.begin();
}

/**
 * @brief Обработчик главной страницы
 */
void handleRoot()
{
    server.send(200, "text/html", getMainPage());
}

/**
 * @brief Обработчик страницы настроек детекции
 */
void handleDetectionSettings()
{
    server.send(200, "text/html", getDetectionSettingsPage());
}

/**
 * @brief Обработчик страницы настроек Wi-Fi
 */
void handleWifiSettings()
{
    server.send(200, "text/html", getWifiSettingsPage());
}

/**
 * @brief Обработчик страницы настроек ROI
 */
void handleROISettings()
{
    server.send(200, "text/html", getROISettingsPage());
}

/**
 * @brief Обработчик сохранения настроек детекции
 */
void handleSaveDetection()
{
    settings.distance = server.arg("distance").toInt();
    settings.interval = server.arg("interval").toInt();
    settings.threshold = server.arg("threshold").toInt();
    settings.area = server.arg("area").toInt();
    settings.dark_min = server.arg("dark_min").toFloat();
    settings.dark_max = server.arg("dark_max").toFloat();
    settings.texture = server.arg("texture").toFloat();
    settings.max_files = server.arg("max_files").toInt();
    server.on("/delete_photo", HTTP_POST, handleDeletePhoto);

    saveSettings();
    server.send(200, "text/plain", "OK");
}

/**
 * @brief Обработчик сохранения настроек Wi-Fi
 */
void handleSaveWifi()
{
    settings.ap_ssid = server.arg("ssid");
    settings.ap_password = server.arg("password");
    saveSettings();
    server.send(200, "text/plain", "OK");
}

/**
 * @brief Обработчик сохранения настроек ROI
 */
void handleSaveROI()
{
    settings.roi_width = server.arg("width").toInt();
    settings.roi_height = server.arg("height").toInt();
    settings.roi_x = server.arg("x").toInt();
    settings.roi_y = server.arg("y").toInt();
    saveSettings();
    updateROICoordinates();
    server.send(200, "text/plain", "OK");
}

/**
 * @brief Обработчик списка фотографий
 */
void handleListPhotos()
{
    server.send(200, "text/html", getPhotosListPage());

    // String fileList = listFiles();
    // server.send(200, "text/html", fileList);
}

/**
 * @brief Обработчик удаления фотографии
 */
void handleDeletePhoto() {
    // // // if (!sd_initialized) {
    // // //     server.send(500, "text/plain", "SD card not available");
    // // //     return;
    // // // }
    
    // // // if (server.hasArg("file")) {
    // // //     String filename = server.arg("file");
        
    // // //     // Удаляем файл фото
    // // //     if (SD_MMC.exists(("/" + filename).c_str())) {
    // // //         SD_MMC.remove(("/" + filename).c_str());
            
    // // //         // Пытаемся удалить соответствующий JSON файл
    // // //         String jsonFilename = filename;
    // // //         if (jsonFilename.endsWith(".jpg") || jsonFilename.endsWith(".JPG")) {
    // // //             jsonFilename = jsonFilename.substring(0, jsonFilename.length() - 4) + ".json";
    // // //         } else if (jsonFilename.endsWith(".jpeg") || jsonFilename.endsWith(".JPEG")) {
    // // //             jsonFilename = jsonFilename.substring(0, jsonFilename.length() - 5) + ".json";
    // // //         }
            
    // // //         if (SD_MMC.exists(("/" + jsonFilename).c_str())) {
    // // //             SD_MMC.remove(("/" + jsonFilename).c_str());
    // // //         }
            
    // // //         server.send(200, "text/plain", "File deleted");
    // // //     } else {
    // // //         server.send(404, "text/plain", "File not found");
    // // //     }
    // // // } else {
    // // //     server.send(400, "text/plain", "Missing file parameter");
    // // // }
}
