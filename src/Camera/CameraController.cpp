/**
 * @file CameraController.cpp
 * @brief Реализация управления камерой ESP32-CAM
 */

#include "Camera/CameraController.hpp"
#include <Arduino.h>

// Внешние объявления
extern bool camera_initialized;

/**
 * @brief Инициализация камеры
 */
bool setupCamera()
{
    camera_config_t config;

    // Конфигурация пинов для AI-Thinker ESP32-CAM
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 10000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Настройка в зависимости от наличия PSRAM
    if (psramFound())
    {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_QQVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        camera_initialized = false;
        return false;
    }

    // Дополнительная настройка сенсора
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_QQVGA);
    s->set_pixformat(s, PIXFORMAT_GRAYSCALE);

    camera_initialized = true;
    return true;
}

/**
 * @brief Захват кадра для детекции
 */
camera_fb_t* captureFrame()
{
    return esp_camera_fb_get();
}

/**
 * @brief Захват высококачественного кадра для сохранения
 */
camera_fb_t* captureHighResFrame()
{
    return esp_camera_fb_get();
}

/**
 * @brief Освобождение буфера кадра
 */
void releaseFrame(camera_fb_t* fb)
{
    esp_camera_fb_return(fb);
}

/**
 * @brief Переключение в режим детекции
 */
void switchToDetectionMode()
{
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_QQVGA);
    s->set_pixformat(s, PIXFORMAT_GRAYSCALE);
}

/**
 * @brief Переключение в режим фотографии
 */
void switchToPhotoMode()
{
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_SVGA);
    s->set_pixformat(s, PIXFORMAT_JPEG);
}