/**
 * @file PreferencesManager.hpp
 * @brief Управление данными в NVS памяти
 */

#ifndef PREFERENCES_MANAGER_HPP
#define PREFERENCES_MANAGER_HPP

#include <Preferences.h>

// Прототипы функций
void setupPreferences();
void loadPreferences();
void savePreferences();

#endif // PREFERENCES_MANAGER_HPP