/**
 * @file FlashController.cpp
 * @brief Реализация управления вспышкой
 */

#include "Utils/FlashController.hpp"

/**
 * @brief Инициализация пина вспышки
 */
void setupFlash()
{
    pinMode(FLASH_GPIO_NUM, OUTPUT);
    digitalWrite(FLASH_GPIO_NUM, LOW);
}

/**
 * @brief Включение вспышки
 */
void onFlash()
{
    digitalWrite(FLASH_GPIO_NUM, HIGH);
}

/**
 * @brief Выключение вспышки
 */
void offFlash()
{
    digitalWrite(FLASH_GPIO_NUM, LOW);
}

/**
 * @brief Переключение состояния вспышки
 */
void reverseFlash()
{
    digitalWrite(FLASH_GPIO_NUM, !digitalRead(FLASH_GPIO_NUM));
}