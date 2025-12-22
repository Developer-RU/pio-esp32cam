/**
 * @file DistanceSensor.cpp
 * @brief Реализация работы с датчиком расстояния
 */

#include "Sensors/DistanceSensor.hpp"
#include <ArduinoJson.h>

/**
 * @brief Измерение расстояния
 */
uint16_t measure()
{
    static int previous_valid_distance = 0;
    String json = "";

    if (Serial.available())
    {
        delay(50);

        while (Serial.available())
        {
            char val = Serial.read();
            json += val;
            delay(2);
        }

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, json);

        if (!error)
        {
            previous_valid_distance = doc["medium"] | 0;
        }
    }

    previous_valid_distance /= 10;
    return previous_valid_distance;
}