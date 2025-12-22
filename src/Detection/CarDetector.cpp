/**
 * @file CarDetector.cpp
 * @brief Реализация логики детектирования автомобилей
 */

#include "Detection/CarDetector.hpp"
#include "Config/Config.hpp"
#include "Camera/CameraController.hpp"
#include "Storage/SDCardManager.hpp"
#include "Storage/PreferencesManager.hpp"
#include "Utils/FlashController.hpp"
#include <ArduinoJson.h>

// Внешние объявления
extern bool car_detected;
extern int lastDistance;
extern int resDistance;
extern unsigned long timeInterval;
extern int photoNumber;
extern bool sd_initialized;
extern Settings settings;

/**
 * @brief Основная функция детектирования автомобиля
 */
void detectCar()
{
    camera_fb_t *fb = captureFrame();

    if (fb)
    {
        int darkPixels = 0;
        int totalPixels = 0;
        float darkRatio = 0;

        if (fb->format == PIXFORMAT_GRAYSCALE)
        {
            analyzeFrame(fb, darkPixels, totalPixels, darkRatio);
        }

        releaseFrame(fb);

        if (car_detected)
        {
            takeHighQualityPhoto(darkPixels, totalPixels, darkRatio);
        }
    }
    else
    {
        Serial.println("Camera capture failed");
    }
}

/**
 * @brief Анализ кадра для детектирования
 */
int analyzeFrame(camera_fb_t *fb, int &darkPixels, int &totalPixels, float &darkRatio)
{
    uint8_t *grayImage = fb->buf;

    int max_y = min(settings.roi_y + settings.roi_height, (int)fb->height);
    int max_x = min(settings.roi_x + settings.roi_width, (int)fb->width);
    int roi_w = max(0, max_x - settings.roi_x);
    int roi_h = max(0, max_y - settings.roi_y);
    totalPixels = roi_w * roi_h;

    for (int y = settings.roi_y; y < max_y; y++)
    {
        for (int x = settings.roi_x; x < max_x; x++)
        {
            if (x >= fb->width || y >= fb->height) continue;
            
            int idx = y * fb->width + x;
            if (grayImage[idx] < settings.threshold)
            {
                darkPixels++;
            }
        }
    }

    darkRatio = totalPixels > 0 ? ((float)darkPixels / totalPixels) : 0.0;

    if (darkRatio > settings.dark_min && darkRatio < settings.dark_max)
    {
        if (darkPixels > settings.area && 
            lastDistance != 0 && 
            settings.distance > lastDistance)
        {
            resDistance = lastDistance;
            car_detected = true;
            Serial.printf("Car detected! Dark pixels: %d, ratio: %.2f, distance: %d\n", 
                         darkPixels, darkRatio, lastDistance);
        }
        else
        {
            Serial.printf("Car not detected! Dark pixels: %d, ratio: %.2f, distance: %d\n", 
                         darkPixels, darkRatio, lastDistance);
        }
    }
    else
    {
        Serial.printf("Car not detected 2! Dark pixels: %d, ratio: %.2f, distance: %d\n", 
                     darkPixels, darkRatio, lastDistance);
    }

    return darkPixels;
}

/**
 * @brief Создание высококачественной фотографии
 */
void takeHighQualityPhoto(int darkPixels, int totalPixels, float darkRatio)
{
    Serial.println("Taking high quality photo...");

    onFlash();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    offFlash();
    vTaskDelay(200 / portTICK_PERIOD_MS);

    switchToPhotoMode();
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    camera_fb_t *hi_res_fb = captureHighResFrame();

    if (hi_res_fb)
    {
        Serial.printf("Captured high-res frame: %zu bytes, format: %d\n", 
                     hi_res_fb->len, hi_res_fb->format);

        if (hi_res_fb->format == PIXFORMAT_JPEG && hi_res_fb->len > 0)
        {
            if (sd_initialized)
            {
                String num = (String)photoNumber;
                while (num.length() < 5)
                    num = "0" + num;

                String filename = "/car_" + num;

                DynamicJsonDocument doc(1024);
                doc["id"] = num;
                doc["image"] = filename + "jpg";
                doc["totalPixels"] = totalPixels;
                doc["darkPixels"] = darkPixels;
                doc["whitePixels"] = totalPixels - darkPixels;
                doc["darkRatio"] = darkRatio;
                doc["distance"] = lastDistance;

                if (savePhotoToSD(filename.c_str(), hi_res_fb, doc))
                {
                    Serial.println("Photo saved successfully: " + filename);
                    savePreferences();
                    vTaskDelay(250 / portTICK_PERIOD_MS);
                }
                else
                {
                    Serial.println("Failed to save photo");
                }
            }
        }
        else
        {
            Serial.println("Invalid frame format or empty frame");
        }
        
        releaseFrame(hi_res_fb);
    }
    else
    {
        Serial.println("High resolution camera capture failed");
    }

    vTaskDelay(250 / portTICK_PERIOD_MS);
    switchToDetectionMode();
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    timeInterval = millis();
}