/**
 * @file FlashController.hpp
 * @brief Управление вспышкой/светодиодом
 */

#ifndef FLASH_CONTROLLER_HPP
#define FLASH_CONTROLLER_HPP

#include <Arduino.h>

#define FLASH_GPIO_NUM 4

// Прототипы функций
void setupFlash();
void onFlash();
void offFlash();
void reverseFlash();

#endif // FLASH_CONTROLLER_HPP