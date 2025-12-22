/**
 * @file Main.cpp
 * @brief Главный файл системы детектирования автомобилей на ESP32-CAM
 */

#include "Main.hpp"
#include "Config/Config.hpp"
#include "Camera/CameraController.hpp"
#include "Storage/SDCardManager.hpp"
#include "Storage/PreferencesManager.hpp"
#include "Web/WebServerManager.hpp"
#include "Sensors/DistanceSensor.hpp"
#include "Detection/CarDetector.hpp"
#include "Utils/FlashController.hpp"

// Глобальные объекты
Preferences preferences;
WebServer server(80);
Settings settings;

// Глобальные переменные состояния
int lastDistance = 0;
int resDistance = 0;
int photoNumber = 0;
bool sd_initialized = false;
bool camera_initialized = false;
bool car_detected = false;
unsigned long timeInterval = 0;
unsigned long timeblink = 0;

/**
 * @brief Функция инициализации системы
 */
void setup()
{
    Serial.begin(9600);
    delay(5000);

    // Инициализация компонентов
    setupFlash();
    setupPreferences();
    setupCamera();
    setupSDCard();
    loadSettings();
    updateROICoordinates();

    // Запуск точки доступа Wi-Fi
    WiFi.softAP(settings.ap_ssid.c_str(), settings.ap_password.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    // Настройка веб-сервера
    setupWebServer();

    delay(500);
    timeInterval = millis();
    Serial.println("System started");
}

/**
 * @brief Главный цикл программы
 */
void loop()
{
    lastDistance = measure();

    if (car_detected == true)
    {
        if (lastDistance > resDistance + 50)
        {
            offFlash();
            car_detected = false;
        }
        else
        {
            if (millis() > timeblink + 500)
            {
                reverseFlash();
                timeblink = millis();
            }
        }
    }
    else
    {
        if (WiFi.softAPgetStationNum() > 0)
        {
            server.handleClient();
            delay(10);
        }
        else
        {
            if (millis() > timeInterval + settings.interval)
            {
                detectCar();
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    server.handleClient();
    vTaskDelay(10 / portTICK_PERIOD_MS);
}