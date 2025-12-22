/**
 * @file CarDetector.hpp
 * @brief Логика детектирования автомобилей
 */

#ifndef CAR_DETECTOR_HPP
#define CAR_DETECTOR_HPP

#include <esp_camera.h>

// Прототипы функций
void detectCar();
int analyzeFrame(camera_fb_t *fb, int &darkPixels, int &totalPixels, float &darkRatio);
void takeHighQualityPhoto(int darkPixels, int totalPixels, float darkRatio);

#endif // CAR_DETECTOR_HPP