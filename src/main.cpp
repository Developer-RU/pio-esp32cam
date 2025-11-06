#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Глобальные настройки
struct Settings
{
  int threshold = 60;
  int area = 500;
  float dark_min = 0.2;
  float dark_max = 0.8;
  float texture = 500.0;
  int max_files = 50;
  int roi_width = 160;
  int roi_height = 120;
  int roi_x = 40; // Центрируем ROI по умолчанию
  int roi_y = 30; // Центрируем ROI по умолчанию
  String ap_ssid = "CarDetector";
  String ap_password = "12345678";
};

Settings settings;

bool sd_initialized = false;
bool camera_initialized = false;

WebServer server(80);

// Прототипы функций
void webServerTask(void *parameter);
void carDetection();
void setupCamera();
void setupSDCard();
void loadSettings();
void saveSettings();
bool savePhotoToSD(const char *filename, camera_fb_t *fb);
String getMainPage();
String getDetectionSettingsPage();
String getWifiSettingsPage();
String getROISettingsPage();
void updateROICoordinates();

void setup()
{
  Serial.begin(115200);

  delay(3000);

  // Инициализация камеры и SD карты
  setupCamera();
  setupSDCard();
  loadSettings();

  // Обновляем координаты ROI на основе текущих настроек
  updateROICoordinates();

  // Запуск точки доступа
  WiFi.softAP(settings.ap_ssid.c_str(), settings.ap_password.c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Запуск задач FreeRTOS
  xTaskCreatePinnedToCore(
      webServerTask,
      "Web Server",
      8192,
      NULL,
      1,
      NULL,
      0);

  // xTaskCreatePinnedToCore(
  //     cameraDetectionTask,
  //     "Camera Detection",
  //     4096,
  //     NULL,
  //     2,
  //     NULL,
  //     1);

  Serial.println("System started");
}

void webServerTask(void *parameter)
{
  // Настройка веб-сервера
  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html", getMainPage()); });

  server.on("/detection", HTTP_GET, []()
            { server.send(200, "text/html", getDetectionSettingsPage()); });

  server.on("/wifi", HTTP_GET, []()
            { server.send(200, "text/html", getWifiSettingsPage()); });

  server.on("/roi", HTTP_GET, []()
            { server.send(200, "text/html", getROISettingsPage()); });

  server.on("/save_detection", HTTP_POST, []()
            {
    settings.threshold = server.arg("threshold").toInt();
    settings.area = server.arg("area").toInt();
    settings.dark_min = server.arg("dark_min").toFloat();
    settings.dark_max = server.arg("dark_max").toFloat();
    settings.texture = server.arg("texture").toFloat();
    settings.max_files = server.arg("max_files").toInt();
    saveSettings();
    server.send(200, "text/plain", "OK"); });

  server.on("/save_wifi", HTTP_POST, []()
            {
    settings.ap_ssid = server.arg("ssid");
    settings.ap_password = server.arg("password");
    saveSettings();
    server.send(200, "text/plain", "OK"); });

  server.on("/save_roi", HTTP_POST, []()
            {
    settings.roi_width = server.arg("width").toInt();
    settings.roi_height = server.arg("height").toInt();
    settings.roi_x = server.arg("x").toInt();
    settings.roi_y = server.arg("y").toInt();
    saveSettings();
    updateROICoordinates(); // Обновляем координаты после сохранения
    server.send(200, "text/plain", "OK"); });

  server.on("/list_photos", HTTP_GET, []()
            {
    if (!sd_initialized) {
      server.send(500, "text/plain", "SD Card not available");
      return;
    }
    
    String fileList = "<html><head><title>Saved Photos</title></head><body>";
    fileList += "<h2>Saved Photos:</h2><ul>";
    File root = SD_MMC.open("/");
    File file = root.openNextFile();
    int count = 0;
    
    while(file && count < 20) { // Ограничиваем список 20 файлами
      if(!file.isDirectory() && String(file.name()).endsWith(".jpg")) {
        fileList += "<li>" + String(file.name()) + " (" + String(file.size()) + " bytes)</li>";
        count++;
      }
      file = root.openNextFile();
    }
    fileList += "</ul>";
    fileList += "<p><a href='/'>Back to main page</a></p>";
    fileList += "</body></html>";
    server.send(200, "text/html", fileList); });

  server.begin();

  while (true)
  {
    server.handleClient();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void carDetection()
{
  digitalWrite(4, LOW);

  camera_fb_t *fb = esp_camera_fb_get();

  if (fb)
  {
    bool car_detected = false;

    // // Анализ только если кадр в градациях серого
    // if (fb->format == PIXFORMAT_GRAYSCALE)
    // {
    //   uint8_t *grayImage = fb->buf;

    //   // Анализ ROI области
    //   int darkPixels = 0;
    //   int totalPixels = settings.roi_width * settings.roi_height;

    //   // Проверяем границы ROI
    //   int max_y = min(settings.roi_y + settings.roi_height, fb->height);
    //   int max_x = min(settings.roi_x + settings.roi_width, fb->width);

    //   for (int y = settings.roi_y; y < max_y; y++)
    //   {
    //     for (int x = settings.roi_x; x < max_x; x++)
    //     {
    //       int idx = y * fb->width + x;
    //       if (grayImage[idx] < settings.threshold)
    //       {
    //         darkPixels++;
    //       }
    //     }
    //   }
    // }

    // Анализ только если кадр в градациях серого
    if (fb->format == PIXFORMAT_GRAYSCALE)
    {
      uint8_t *grayImage = fb->buf;

      // Анализ ROI области
      int darkPixels = 0;
      int totalPixels = settings.roi_width * settings.roi_height;

      for (int y = settings.roi_y; y < settings.roi_y + settings.roi_height; y++)
      {
        for (int x = settings.roi_x; x < settings.roi_x + settings.roi_width; x++)
        {
          int idx = y * fb->width + x;
          if (grayImage[idx] < settings.threshold)
          {
            darkPixels++;
          }
        }
      }

      float darkRatio = (float)darkPixels / totalPixels;

      // Простая проверка условий
      if (darkRatio > settings.dark_min && darkRatio < settings.dark_max)
      {
        if (darkPixels > settings.area)
        {
          car_detected = true;
          Serial.printf("Car detected! Dark pixels: %d, ratio: %.2f\n", darkPixels, darkRatio);
        }
        else
        {
          Serial.printf("Car not detected! Dark pixels: %d, ratio: %.2f\n", darkPixels, darkRatio);
        }
      }
      else
      {
        Serial.printf("Car not detected 2! Dark pixels: %d, ratio: %.2f\n", darkPixels, darkRatio);
      }
    }

    esp_camera_fb_return(fb);

    // Если обнаружена машина - делаем качественный снимок
    if (car_detected)
    {
      Serial.println("Taking high quality photo...");

      delay(250);

      sensor_t *s = esp_camera_sensor_get();

      s->set_framesize(s, FRAMESIZE_VGA);
      s->set_pixformat(s, PIXFORMAT_JPEG);

      // s->set_quality(s, 12); // Высокое качество (меньше число = лучше качество)

      // Даем камере время на перестройку
      delay(1500);

      camera_fb_t *hi_res_fb = esp_camera_fb_get();

      if (hi_res_fb)
      {
        Serial.printf("Captured high-res frame: %zu bytes, format: %d\n", hi_res_fb->len, hi_res_fb->format);

        if (hi_res_fb->format == PIXFORMAT_JPEG && hi_res_fb->len > 0)
        {
          if (sd_initialized)
          {
            // Генерируем имя файла с временной меткой
            String filename = "/car_" + String(millis()) + ".jpg";
            if (savePhotoToSD(filename.c_str(), hi_res_fb))
            {
              delay(250);
              Serial.println("Photo saved successfully: " + filename);
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
        esp_camera_fb_return(hi_res_fb);
      }
      else
      {
        Serial.println("High resolution camera capture failed");
      }

      delay(250);

      s->set_framesize(s, FRAMESIZE_QQVGA);
      s->set_pixformat(s, PIXFORMAT_GRAYSCALE);

      delay(1500);
    }
  }
  else
  {
    Serial.println("Camera capture failed");

    // s->set_framesize(s, FRAMESIZE_QQVGA);
    // s->set_pixformat(s, PIXFORMAT_GRAYSCALE);

    // delay(1500);

    // digitalWrite(PWDN_GPIO_NUM, !digitalRead(PWDN_GPIO_NUM));
    // delay(50);
    // digitalWrite(PWDN_GPIO_NUM, !digitalRead(PWDN_GPIO_NUM));

    // setupCamera();

    // s->set_quality(s, 12); // Высокое качество (меньше число = лучше качество)
  }

  // }
}

bool savePhotoToSD(const char *filename, camera_fb_t *fb)
{
  if (!sd_initialized || !fb || fb->format != PIXFORMAT_JPEG)
  {
    Serial.println("Invalid parameters for photo saving");
    return false;
  }

  Serial.printf("Saving photo: %s, size: %zu bytes\n", filename, fb->len);

  File file = SD_MMC.open(filename, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return false;
  }

  size_t written = file.write(fb->buf, fb->len);
  file.close();

  if (written != fb->len)
  {
    Serial.printf("Write failed: %zu of %zu bytes written\n", written, fb->len);
    return false;
  }

  // Проверяем, что файл создан и имеет правильный размер
  file = SD_MMC.open(filename, FILE_READ);
  if (!file)
  {
    Serial.println("Cannot verify saved file");
    return false;
  }

  size_t fileSize = file.size();
  file.close();

  if (fileSize != fb->len)
  {
    Serial.printf("File size mismatch: %zu vs %zu\n", fileSize, fb->len);
    return false;
  }

  Serial.printf("Photo saved successfully: %s (%zu bytes)\n", filename, fileSize);
  return true;
}

void setupCamera()
{
  camera_config_t config;
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

  if (psramFound())
  {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();

  s->set_framesize(s, FRAMESIZE_QQVGA);
  s->set_pixformat(s, PIXFORMAT_GRAYSCALE);

  // // // // // Дополнительная настройка сенсора
  // // // // sensor_t *s = esp_camera_sensor_get();
  // // // // if (s != NULL)
  // // // // {
  // // // //   // Настройки для лучшего качества
  // // // //   s->set_brightness(s, 0);                 // -2 to 2
  // // // //   s->set_contrast(s, 0);                   // -2 to 2
  // // // //   s->set_saturation(s, 0);                 // -2 to 2
  // // // //   s->set_special_effect(s, 0);             // 0 to 6 (0 - No Effect)
  // // // //   s->set_whitebal(s, 1);                   // 0 = disable , 1 = enable
  // // // //   s->set_awb_gain(s, 1);                   // 0 = disable , 1 = enable
  // // // //   s->set_wb_mode(s, 0);                    // 0 to 4 - if awb_gain enabled (0 - Auto)
  // // // //   s->set_exposure_ctrl(s, 1);              // 0 = disable , 1 = enable
  // // // //   s->set_aec2(s, 0);                       // 0 = disable , 1 = enable
  // // // //   s->set_ae_level(s, 0);                   // -2 to 2
  // // // //   s->set_aec_value(s, 300);                // 0 to 1200
  // // // //   s->set_gain_ctrl(s, 1);                  // 0 = disable , 1 = enable
  // // // //   s->set_agc_gain(s, 0);                   // 0 to 30
  // // // //   s->set_gainceiling(s, (gainceiling_t)0); // 0 to 6
  // // // //   s->set_bpc(s, 0);                        // 0 = disable , 1 = enable
  // // // //   s->set_wpc(s, 1);                        // 0 = disable , 1 = enable
  // // // //   s->set_raw_gma(s, 1);                    // 0 = disable , 1 = enable
  // // // //   s->set_lenc(s, 1);                       // 0 = disable , 1 = enable
  // // // //   s->set_hmirror(s, 0);                    // 0 = disable , 1 = enable
  // // // //   s->set_vflip(s, 0);                      // 0 = disable , 1 = enable
  // // // //   s->set_dcw(s, 1);                        // 0 = disable , 1 = enable
  // // // //   s->set_colorbar(s, 0);                   // 0 = disable , 1 = enable

  // // // //   // s->set_framesize(s, FRAMESIZE_QQVGA);
  // // // //   // s->set_pixformat(s, PIXFORMAT_GRAYSCALE);
}

void setupSDCard()
{
  // Для AI-Thinker ESP32-CAM
  if (!SD_MMC.begin("/sdcard", true))
  {
    Serial.println("SD Card Mount Failed");
    sd_initialized = false;
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    sd_initialized = false;
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  sd_initialized = true;
  Serial.println("SD Card initialized successfully");
}

void loadSettings()
{
  if (!sd_initialized)
  {
    Serial.println("No SD card, using default settings");
    return;
  }

  File file = SD_MMC.open("/settings.json");
  if (!file)
  {
    Serial.println("No settings file found, using defaults");
    return;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (!error)
  {
    settings.threshold = doc["threshold"] | 60;
    settings.area = doc["area"] | 500;
    settings.dark_min = doc["dark_min"] | 0.2;
    settings.dark_max = doc["dark_max"] | 0.8;
    settings.texture = doc["texture"] | 500.0;
    settings.max_files = doc["max_files"] | 50;
    settings.roi_width = doc["roi_width"] | 160;
    settings.roi_height = doc["roi_height"] | 120;
    settings.roi_x = doc["roi_x"] | 40;
    settings.roi_y = doc["roi_y"] | 30;
    settings.ap_ssid = doc["ap_ssid"] | "CarDetector";
    settings.ap_password = doc["ap_password"] | "12345678";
    Serial.println("Settings loaded from SD card");
  }
  else
  {
    Serial.println("Error loading settings, using defaults");
  }
}

void saveSettings()
{
  if (!sd_initialized)
    return;

  File file = SD_MMC.open("/settings.json", FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open settings file for writing");
    return;
  }

  DynamicJsonDocument doc(1024);
  doc["threshold"] = settings.threshold;
  doc["area"] = settings.area;
  doc["dark_min"] = settings.dark_min;
  doc["dark_max"] = settings.dark_max;
  doc["texture"] = settings.texture;
  doc["max_files"] = settings.max_files;
  doc["roi_width"] = settings.roi_width;
  doc["roi_height"] = settings.roi_height;
  doc["roi_x"] = settings.roi_x;
  doc["roi_y"] = settings.roi_y;
  doc["ap_ssid"] = settings.ap_ssid;
  doc["ap_password"] = settings.ap_password;

  if (serializeJson(doc, file) == 0)
  {
    Serial.println("Failed to write settings");
  }
  else
  {
    Serial.println("Settings saved successfully");
  }
  file.close();
}

void updateROICoordinates()
{
  // Центрируем ROI если координаты не заданы
  if (settings.roi_x == 0 && settings.roi_y == 0)
  {
    settings.roi_x = (160 - settings.roi_width) / 2;
    settings.roi_y = (120 - settings.roi_height) / 2;
  }

  // Ограничиваем ROI размерами изображения
  settings.roi_x = max(0, min(settings.roi_x, 160 - settings.roi_width));
  settings.roi_y = max(0, min(settings.roi_y, 120 - settings.roi_height));

  Serial.printf("ROI configured: x=%d, y=%d, width=%d, height=%d\n",
                settings.roi_x, settings.roi_y, settings.roi_width, settings.roi_height);
}

// HTML страницы
String getMainPage()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Car Detector</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <div class="nav">
        <a href="/">Main</a>
        <a href="/detection">Detection Settings</a>
        <a href="/wifi">WiFi Settings</a>
        <a href="/roi">ROI Settings</a>
        <a href="/list_photos">View Photos</a>
    </div>
    
    <div class="card">
        <h2>Car Detection System</h2>
        <div class="status )rawliteral" +
                String(camera_initialized ? "status-ok" : "status-error") + R"rawliteral(">
            Camera: )rawliteral" +
                String(camera_initialized ? "OK" : "ERROR") + R"rawliteral(
        </div>
        <div class="status )rawliteral" +
                String(sd_initialized ? "status-ok" : "status-error") + R"rawliteral(">
            SD Card: )rawliteral" +
                String(sd_initialized ? "OK" : "ERROR") + R"rawliteral(
        </div>
        <p>IP Address: )rawliteral" +
                WiFi.softAPIP().toString() + R"rawliteral(</p>
        <p>Connected clients: )rawliteral" +
                String(WiFi.softAPgetStationNum()) + R"rawliteral(</p>
    </div>
    
    <div class="card">
        <h3>Quick Actions</h3>
        <button class="btn" onclick="location.href='/detection'">Adjust Sensitivity</button>
        <button class="btn" onclick="location.href='/roi'">Configure Detection Area</button>
        <button class="btn" onclick="location.href='/list_photos'">View Saved Photos</button>
    </div>
</body>
</html>
)rawliteral";
  return html;
}

String getDetectionSettingsPage()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Detection Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; margin: 5px; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .slider { width: 100%; }
        .value-display { font-weight: bold; color: #007bff; }
    </style>
</head>
<body>
    <div class="nav">
        <a href="/">Main</a>
        <a href="/detection">Detection Settings</a>
        <a href="/wifi">WiFi Settings</a>
        <a href="/roi">ROI Settings</a>
        <a href="/list_photos">View Photos</a>
    </div>
    
    <div class="card">
        <h2>Detection Sensitivity Settings</h2>
        
        <form id="detectionForm">
            <label>Threshold (0-255):</label>
            <input type="range" class="slider" id="threshold" min="0" max="255" value=")rawliteral" +
                String(settings.threshold) + R"rawliteral(">
            <span class="value-display" id="thresholdValue">)rawliteral" +
                String(settings.threshold) + R"rawliteral(</span>
            
            <label>Minimum Area:</label>
            <input type="number" id="area" value=")rawliteral" +
                String(settings.area) + R"rawliteral(" min="1" max="10000">
            
            <label>Dark Pixel Min Ratio (0-1):</label>
            <input type="number" id="dark_min" step="0.01" value=")rawliteral" +
                String(settings.dark_min) + R"rawliteral(" min="0" max="1">
            
            <label>Dark Pixel Max Ratio (0-1):</label>
            <input type="number" id="dark_max" step="0.01" value=")rawliteral" +
                String(settings.dark_max) + R"rawliteral(" min="0" max="1">
            
            <label>Texture Sensitivity:</label>
            <input type="number" id="texture" value=")rawliteral" +
                String(settings.texture) + R"rawliteral(" min="0" max="10000">
            
            <label>Max Files:</label>
            <input type="number" id="max_files" value=")rawliteral" +
                String(settings.max_files) + R"rawliteral(" min="1" max="1000">
            
            <button type="button" class="btn" onclick="saveSettings()">Save Settings</button>
        </form>
    </div>

    <script>
        document.getElementById('threshold').oninput = function() {
            document.getElementById('thresholdValue').textContent = this.value;
        }
        
        async function saveSettings() {
            const formData = new FormData();
            formData.append('threshold', document.getElementById('threshold').value);
            formData.append('area', document.getElementById('area').value);
            formData.append('dark_min', document.getElementById('dark_min').value);
            formData.append('dark_max', document.getElementById('dark_max').value);
            formData.append('texture', document.getElementById('texture').value);
            formData.append('max_files', document.getElementById('max_files').value);
            
            try {
                const response = await fetch('/save_detection', { method: 'POST', body: formData });
                if (response.ok) {
                    alert('Settings saved successfully!');
                } else {
                    alert('Error saving settings!');
                }
            } catch (error) {
                alert('Error saving settings: ' + error);
            }
        }
    </script>
</body>
</html>
)rawliteral";
  return html;
}

String getWifiSettingsPage()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>WiFi Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
    </style>
</head>
<body>
    <div class="nav">
        <a href="/">Main</a>
        <a href="/detection">Detection Settings</a>
        <a href="/wifi">WiFi Settings</a>
        <a href="/roi">ROI Settings</a>
        <a href="/list_photos">View Photos</a>
    </div>
    
    <div class="card">
        <h2>WiFi Access Point Settings</h2>
        <p><strong>Current AP:</strong> )rawliteral" +
                settings.ap_ssid + R"rawliteral(</p>
        <p><strong>Current Password:</strong> )rawliteral" +
                settings.ap_password + R"rawliteral(</p>
        
        <form id="wifiForm">
            <label>SSID:</label>
            <input type="text" id="ssid" value=")rawliteral" +
                settings.ap_ssid + R"rawliteral(" required>
            
            <label>Password:</label>
            <input type="password" id="password" value=")rawliteral" +
                settings.ap_password + R"rawliteral(" required minlength="8">
            
            <button type="button" class="btn" onclick="saveSettings()">Save WiFi Settings</button>
        </form>
        
        <p><em>Note: Changes will take effect after reboot</em></p>
    </div>

    <script>
        async function saveSettings() {
            const formData = new FormData();
            formData.append('ssid', document.getElementById('ssid').value);
            formData.append('password', document.getElementById('password').value);
            
            try {
                const response = await fetch('/save_wifi', { method: 'POST', body: formData });
                if (response.ok) {
                    alert('WiFi settings saved! Please reboot the device.');
                } else {
                    alert('Error saving WiFi settings!');
                }
            } catch (error) {
                alert('Error saving WiFi settings: ' + error);
            }
        }
    </script>
</body>
</html>
)rawliteral";
  return html;
}

String getROISettingsPage()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ROI Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
    </style>
</head>
<body>
    <div class="nav">
        <a href="/">Main</a>
        <a href="/detection">Detection Settings</a>
        <a href="/wifi">WiFi Settings</a>
        <a href="/roi">ROI Settings</a>
        <a href="/list_photos">View Photos</a>
    </div>
    
    <div class="card">
        <h2>Region of Interest (ROI) Settings</h2>
        <p>Define the rectangular area in the center of the image that will be analyzed for car detection.</p>
        
        <form id="roiForm">
            <label>ROI Width (pixels):</label>
            <input type="number" id="width" value=")rawliteral" +
                String(settings.roi_width) + R"rawliteral(" min="1" max="160">
            
            <label>ROI Height (pixels):</label>
            <input type="number" id="height" value=")rawliteral" +
                String(settings.roi_height) + R"rawliteral(" min="1" max="120">
            
            <label>ROI X Offset:</label>
            <input type="number" id="x" value=")rawliteral" +
                String(settings.roi_x) + R"rawliteral(" min="0" max="160">
            
            <label>ROI Y Offset:</label>
            <input type="number" id="y" value=")rawliteral" +
                String(settings.roi_y) + R"rawliteral(" min="0" max="120">
            
            <button type="button" class="btn" onclick="saveSettings()">Save ROI Settings</button>
        </form>
        
        <p><em>Note: Detection image is 160x120 pixels. ROI must fit within these dimensions.</em></p>
        <p><strong>Current ROI:</strong> X=)rawliteral" +
                String(settings.roi_x) + R"rawliteral(, Y=)rawliteral" + String(settings.roi_y) + R"rawliteral(, Width=)rawliteral" + String(settings.roi_width) + R"rawliteral(, Height=)rawliteral" + String(settings.roi_height) + R"rawliteral(</p>
    </div>

    <script>
        async function saveSettings() {
            const formData = new FormData();
            formData.append('width', document.getElementById('width').value);
            formData.append('height', document.getElementById('height').value);
            formData.append('x', document.getElementById('x').value);
            formData.append('y', document.getElementById('y').value);
            
            try {
                const response = await fetch('/save_roi', { method: 'POST', body: formData });
                if (response.ok) {
                    alert('ROI settings saved successfully!');
                } else {
                    alert('Error saving ROI settings!');
                }
            } catch (error) {
                alert('Error saving ROI settings: ' + error);
            }
        }
    </script>
</body>
</html>
)rawliteral";
  return html;
}

void loop()
{
  carDetection();
  // Основной цикл не используется - все задачи в FreeRTOS
  delay(1000); // Проверка каждую секунду
}