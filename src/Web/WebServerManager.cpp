/**
 * @file WebServerManager.cpp
 * @brief Реализация веб-сервера
 */

 #include <Arduino.h>

#include "Web/WebServerManager.hpp"
#include "Web/HtmlPages.hpp"
#include "Config/Config.hpp"
#include "Storage/SDCardManager.hpp"
#include <esp_camera.h>

// Внешние объявления
extern WebServer server;
extern Settings settings;

extern bool camera_initialized;
extern bool sd_initialized;

// Глобальные переменные для видеопотока
// extern camera_fb_t* fb;
// extern TaskHandle_t cameraTaskHandle;


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

    // Новые эндпоинты для видеопотока и файлов
    // server.on("/stream", HTTP_GET, handleStream);
    server.on("/capture", HTTP_GET, handleCapture);

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

/**
 * @brief Обработчик видеопотока (MJPEG)
 */
void handleStream()
{
    if (!camera_initialized) {
        server.send(503, "text/plain", "Camera not initialized");
        return;
    }
    
    WiFiClient client = server.client();
    
    // Устанавливаем заголовки для MJPEG потока
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "\r\n";
    client.print(response);
        
    sensor_t *s = esp_camera_sensor_get();
    // s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_pixformat(s, PIXFORMAT_JPEG);

    while (client.connected()) {
        // Захват кадра с камеры
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            break;
        }
        
        // Формируем часть multipart ответа
        client.print("--frame\r\n");
        client.print("Content-Type: image/jpeg\r\n");
        client.printf("Content-Length: %d\r\n", fb->len);
        client.print("\r\n");
        client.write(fb->buf, fb->len);
        client.print("\r\n");
        
        esp_camera_fb_return(fb);
        
        // Небольшая задержка для управления FPS
        delay(100); // ~10 FPS
    }
}

/**
 * @brief Обработчик захвата одного кадра
 */
// // // void handleCapture()
// // // {
// // //     if (!camera_initialized) {
// // //         server.send(503, "text/plain", "Camera not initialized");
// // //         return;
// // //     }
    
// // //     sensor_t *s = esp_camera_sensor_get();
// // //     // s->set_framesize(s, FRAMESIZE_QQVGA);
// // //     s->set_pixformat(s, PIXFORMAT_JPEG);

// // //     camera_fb_t *fb = esp_camera_fb_get();
// // //     if (!fb) {
// // //         server.send(500, "text/plain", "Camera capture failed");
// // //         return;
// // //     }

// // //     vTaskDelay(200);

// // //     server.sendHeader("Content-Type", "image/jpeg");
// // //     server.sendHeader("Content-Length", String(fb->len));
// // //     server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
// // //     server.sendHeader("Pragma", "no-cache");
// // //     server.sendHeader("Expires", "0");
// // //     // server.send(200, "image/jpeg", (const char*)fb->buf, fb->len);
// // //     server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);

// // //     esp_camera_fb_return(fb);
// // // }

/**
 * @brief Обработчик захвата одного кадра с повторными попытками
 */
// // // void handleCapture() {
// // //     if (!camera_initialized) {
// // //         server.send(503, "text/plain", "Camera not initialized");
// // //         return;
// // //     }
    
// // //     sensor_t *s = esp_camera_sensor_get();
// // //     // s->set_framesize(s, FRAMESIZE_QQVGA);
// // //     s->set_pixformat(s, PIXFORMAT_JPEG);

// // //     camera_fb_t *fb = NULL;
// // //     int attempts = 3; // Количество попыток захвата кадра

// // //     for (int i = 0; i < attempts; i++) {
// // //         fb = esp_camera_fb_get();
// // //         if (fb) {
// // //             break; // Успешно получили кадр
// // //         }
// // //         delay(100); // Небольшая задержка перед повторной попыткой
// // //     }

// // //     if (!fb) {
// // //         server.send(500, "text/plain", "Camera capture failed after multiple attempts");
// // //         return;
// // //     }

// // //     // Проверяем, что длина буфера корректна (минимальный размер для JPEG - например, 100 байт)
// // //     if (fb->len < 100) {
// // //         esp_camera_fb_return(fb);
// // //         server.send(500, "text/plain", "Camera capture returned too small buffer");
// // //         return;
// // //     }

// // //     // Отправляем изображение
// // //     server.sendHeader("Content-Type", "image/jpeg");
// // //     server.sendHeader("Content-Length", String(fb->len));
// // //     server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
// // //     server.sendHeader("Pragma", "no-cache");
// // //     server.sendHeader("Expires", "0");

// // //     // Отправляем данные по частям, если send не поддерживает большие данные
// // //     server.setContentLength(fb->len);
// // //     server.send(200, "image/jpeg", "");
// // //     server.sendContent((const char *)fb->buf, fb->len);

// // //     esp_camera_fb_return(fb);
// // // }


/**
 * @brief Улучшенный обработчик захвата кадра с защитой от битых изображений
 */
void handleCapture()
{
    if (!camera_initialized) {
        server.send(503, "text/plain", "Camera not initialized");
        return;
    }
    
    sensor_t *s = esp_camera_sensor_get();
    s->set_pixformat(s, PIXFORMAT_JPEG);
    
    vTaskDelay(200);

    // Несколько попыток захвата кадра
    camera_fb_t* fb = NULL;
    int attempts = 3;
    
    for (int i = 0; i < attempts; i++) {
        fb = esp_camera_fb_get();
        if (fb != NULL && fb->len > 100) { // Минимальный размер для JPEG
            break;
        }
        if (fb != NULL) {
            esp_camera_fb_return(fb);
            fb = NULL;
        }
        delay(50);
    }
    
    if (fb == NULL || fb->len <= 100) {
        if (fb != NULL) esp_camera_fb_return(fb);
        
        // Возвращаем заглушку вместо ошибки
        const char* placeholder = R"rawliteral(
HTTP/1.1 200 OK
Content-Type: image/svg+xml
Content-Length: 463
Connection: close

<svg xmlns="http://www.w3.org/2000/svg" width="320" height="240" viewBox="0 0 320 240">
  <rect width="100%" height="100%" fill="#3498db"/>
  <text x="50%" y="40%" text-anchor="middle" fill="white" font-size="20" font-family="Arial">Camera Error</text>
  <text x="50%" y="50%" text-anchor="middle" fill="white" font-size="16" font-family="Arial">Please check camera connection</text>
  <rect x="110" y="140" width="100" height="60" fill="none" stroke="white" stroke-width="2"/>
  <circle cx="160" cy="170" r="15" fill="white"/>
</svg>
)rawliteral";
        
        WiFiClient client = server.client();
        client.print(placeholder);
        client.stop();
        return;
    }
    
    // Проверяем, что это валидный JPEG (начинается с FF D8 FF)
    if (fb->len < 4 || fb->buf[0] != 0xFF || fb->buf[1] != 0xD8 || fb->buf[2] != 0xFF) {
        esp_camera_fb_return(fb);
        
        // Возвращаем заглушку для невалидного JPEG
        const char* placeholder = R"rawliteral(
HTTP/1.1 200 OK
Content-Type: image/svg+xml
Content-Length: 380
Connection: close

<svg xmlns="http://www.w3.org/2000/svg" width="320" height="240" viewBox="0 0 320 240">
  <rect width="100%" height="100%" fill="#e74c3c"/>
  <text x="50%" y="45%" text-anchor="middle" fill="white" font-size="20" font-family="Arial">Invalid Image</text>
  <text x="50%" y="55%" text-anchor="middle" fill="white" font-size="16" font-family="Arial">Retrying...</text>
</svg>
)rawliteral";
        
        WiFiClient client = server.client();
        client.print(placeholder);
        client.stop();
        return;
    }
    
    // Отправляем корректный JPEG
    WiFiClient client = server.client();
    String headers = "HTTP/1.1 200 OK\r\n";
    headers += "Content-Type: image/jpeg\r\n";
    headers += "Content-Length: " + String(fb->len) + "\r\n";
    headers += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
    headers += "Pragma: no-cache\r\n";
    headers += "Expires: 0\r\n";
    headers += "Connection: close\r\n";
    headers += "\r\n";
    
    client.print(headers);
    client.write(fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
    client.stop();
}