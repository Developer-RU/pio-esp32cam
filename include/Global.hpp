#ifndef __GLOBAL_FILE__
#define __GLOBAL_FILE__

#include "Arduino.h"

// Глобальные настройки
struct Settings
{
  int distance = 380;
  int interval = 5000;
  int threshold = 160;
  int area = 50;
  float dark_min = 0.2;
  float dark_max = 0.8;
  float texture = 500.0;
  int max_files = 250;
  int roi_width = 80;
  int roi_height = 60;
  int roi_x = 40; // Центрируем ROI по умолчанию
  int roi_y = 30; // Центрируем ROI по умолчанию
  String ap_ssid = "CarDetector";
  String ap_password = "12345678";
};



#endif