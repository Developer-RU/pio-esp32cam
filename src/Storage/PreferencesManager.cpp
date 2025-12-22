/**
 * @file PreferencesManager.cpp
 * @brief Реализация работы с NVS памятью
 */

#include "Storage/PreferencesManager.hpp"

// Внешние объявления
extern Preferences preferences;
extern int photoNumber;

/**
 * @brief Инициализация Preferences
 */
void setupPreferences()
{
    preferences.begin("data", false);
    loadPreferences();
}

/**
 * @brief Загрузка данных из NVS памяти
 */
void loadPreferences()
{
    photoNumber = preferences.getInt("number", 1);
}

/**
 * @brief Сохранение данных в NVS память
 */
void savePreferences()
{
    photoNumber += 1;
    preferences.putInt("number", photoNumber);
}