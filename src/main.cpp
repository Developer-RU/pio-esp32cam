#include "Main.hpp"

Preferences preferences;
WebServer server(80);
Settings settings;

int lastDistance = 0;
int photoNumber = 0;

bool sd_initialized = false;
bool camera_initialized = false;

unsigned long timeInterval = 0;

void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  delay(5000);

  // Инициализация preferences, камеры и SD карты
  setupFlash();
  setupPreferences();
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

  setupWebServer();

  delay(500);

  timeInterval = millis();

  Serial.println("System started");
}

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

    // Serial.println(json);

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

bool savePhotoToSD(const char *filename, camera_fb_t *fb, DynamicJsonDocument doc)
{
  if (!sd_initialized || !fb || fb->format != PIXFORMAT_JPEG)
  {
    Serial.println("Invalid parameters for photo saving");
    return false;
  }

  String pathPhoto = (String)filename + ".jpg";

  Serial.printf("Saving photo: %s, size: %zu bytes\n", pathPhoto.c_str(), fb->len);

  File filePhoto = SD_MMC.open(pathPhoto.c_str(), FILE_WRITE);
  if (!filePhoto)
  {
    Serial.println("Failed to open file for writing");
    return false;
  }

  size_t written = filePhoto.write(fb->buf, fb->len);
  filePhoto.close();

  if (written != fb->len)
  {
    Serial.printf("Write failed: %zu of %zu bytes written\n", written, fb->len);
    return false;
  }

  // Проверяем, что файл создан и имеет правильный размер
  filePhoto = SD_MMC.open(pathPhoto, FILE_READ);
  if (!filePhoto)
  {
    Serial.println("Cannot verify saved file");
    return false;
  }

  size_t fileSize = filePhoto.size();
  filePhoto.close();

  if (fileSize != fb->len)
  {
    Serial.printf("File size mismatch: %zu vs %zu\n", fileSize, fb->len);
    return false;
  }

  Serial.printf("Photo saved successfully: %s\n", pathPhoto.c_str());

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  String pathData = (String)filename + ".json";

  Serial.printf("Saving data: %s\n", pathData.c_str());

  File fileData = SD_MMC.open(pathData.c_str(), FILE_WRITE);
  if (!fileData)
  {
    Serial.println("Failed to open file for writing");
    return false;
  }

  bool res = false;

  if (serializeJson(doc, fileData) == 0)
  {
    Serial.println("Failed to write data");
    res = false;
  }
  else
  {
    Serial.println("Settings saved data");
    res = true;
  }

  fileData.close();

  // Serial.printf("Data saved successfully: %s\n", pathData.c_str());

  return res;
}

void onFlash()
{
  digitalWrite(FLASH_GPIO_NUM, HIGH);
}

void offFlash()
{
  digitalWrite(FLASH_GPIO_NUM, LOW);
}

void setupFlash()
{
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  digitalWrite(FLASH_GPIO_NUM, LOW);
}

void setupWebServer()
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
    settings.distance = server.arg("distance").toInt();
    settings.interval = server.arg("interval").toInt();
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
}

void setupPreferences()
{
  loadPreferences();
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

  camera_initialized = true;

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

void loadPreferences()
{
  preferences.begin("data", false);

  photoNumber = preferences.getInt("number", 1);
  // String storedString = preferences.getString("myString", "Default");
  // bool storedFlag = preferences.getBool("myFlag", false);

  // preferences.end();
}

void savePreferences()
{
  // preferences.begin("data", true);

  photoNumber += 1;

  preferences.putInt("number", photoNumber);
  // preferences.putString("myString", "Hello ESP32");
  // preferences.putBool("myFlag", true);

  // preferences.end();
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

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (!error)
  {
    settings.distance = doc["distance"] | 380;
    settings.interval = doc["interval"] | 5000;
    settings.threshold = doc["threshold"] | 160;
    settings.area = doc["area"] | 500;
    settings.dark_min = doc["dark_min"] | 0.2;
    settings.dark_max = doc["dark_max"] | 0.8;
    settings.texture = doc["texture"] | 500.0;
    settings.max_files = doc["max_files"] | 250;
    settings.roi_width = doc["roi_width"] | 80;
    settings.roi_height = doc["roi_height"] | 60;
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

  DynamicJsonDocument doc(2048);

  doc["distance"] = settings.distance;
  doc["interval"] = settings.interval;
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

    Serial.println(doc.as<String>());
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

  Serial.printf("ROI configured: x=%d, y=%d, width=%d, height=%d\n", settings.roi_x, settings.roi_y, settings.roi_width, settings.roi_height);
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
        .btn { width: 100%; padding: 8px; margin: 5px 0;                 background: #007bff; color: white; border: 1px solid #ddd; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    
    <div class="card">
        <h3>Quick Navigation</h3>

        <button class="btn" onclick="location.href='/'">Home Page</button><br>
        <button class="btn" onclick="location.href='/detection'">Detection Settings</button><br>
        <button class="btn" onclick="location.href='/wifi'">WiFi Settings</button><br>
        <button class="btn" onclick="location.href='/roi'">ROI Settings</button><br>
        <button class="btn" onclick="location.href='/list_photos'">Photos info</button>
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
    <title>Car Detector</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { width: 100%; padding: 8px; margin: 5px 0;                 background: #007bff; color: white; border: 1px solid #ddd; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    
    <div class="card">
        <h3>Quick Navigation</h3>

        <button class="btn" onclick="location.href='/'">Home Page</button><br>
        <button class="btn" onclick="location.href='/detection'">Detection Settings</button><br>
        <button class="btn" onclick="location.href='/wifi'">WiFi Settings</button><br>
        <button class="btn" onclick="location.href='/roi'">ROI Settings</button><br>
        <button class="btn" onclick="location.href='/list_photos'">Photos info</button>
    </div>
    
    <div class="card">
        <h2>Detection Sensitivity Settings</h2>
        
        <form id="detectionForm">

            <label>Distance (0-400): <span class="value-display" id="distanceValue">)rawliteral" +
                String(settings.distance) + R"rawliteral(</span></label>
            <input type="range" class="slider" id="distance" min="0" max="400" value=")rawliteral" +
                String(settings.distance) + R"rawliteral("></input><br>


            <label>Interval detection (0-10000):</label>
            <input type="number" id="interval" step="100" value=")rawliteral" +
                String(settings.interval) + R"rawliteral(" min="0" max="10000">
                
                
            <label>Threshold (0-255): <span class="value-display" id="thresholdValue">)rawliteral" +
                String(settings.threshold) + R"rawliteral(</span></label>
            <input type="range" class="slider" id="threshold" min="0" max="255" value=")rawliteral" +
                String(settings.threshold) + R"rawliteral("></input><br>
            
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
        document.getElementById('distance').oninput = function() {
            document.getElementById('distanceValue').textContent = this.value;
        }

        document.getElementById('threshold').oninput = function() {
            document.getElementById('thresholdValue').textContent = this.value;
        }
        
        async function saveSettings() {
            const formData = new FormData();
            formData.append('distance', document.getElementById('distance').value);
            formData.append('interval', document.getElementById('interval').value);
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
    <title>Car Detector</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { width: 100%; padding: 8px; margin: 5px 0;                 background: #007bff; color: white; border: 1px solid #ddd; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    
    <div class="card">
        <h3>Quick Navigation</h3>

        <button class="btn" onclick="location.href='/'">Home Page</button><br>
        <button class="btn" onclick="location.href='/detection'">Detection Settings</button><br>
        <button class="btn" onclick="location.href='/wifi'">WiFi Settings</button><br>
        <button class="btn" onclick="location.href='/roi'">ROI Settings</button><br>
        <button class="btn" onclick="location.href='/list_photos'">Photos info</button>
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
    <title>Car Detector</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { width: 100%; padding: 8px; margin: 5px 0;                 background: #007bff; color: white; border: 1px solid #ddd; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    
    <div class="card">
        <h3>Quick Navigation</h3>

        <button class="btn" onclick="location.href='/'">Home Page</button><br>
        <button class="btn" onclick="location.href='/detection'">Detection Settings</button><br>
        <button class="btn" onclick="location.href='/wifi'">WiFi Settings</button><br>
        <button class="btn" onclick="location.href='/roi'">ROI Settings</button><br>
        <button class="btn" onclick="location.href='/list_photos'">Photos info</button>
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
  lastDistance = measure();

  if (WiFi.softAPgetStationNum() > 0)
  {
    server.handleClient();
    delay(10);
  }
  else
  {
    if (millis() > timeInterval + settings.interval)
    {
      camera_fb_t *fb = esp_camera_fb_get();

      if (fb)
      {
        bool car_detected = false;

        int darkPixels = 0;
        int totalPixels = 0;
        float darkRatio = 0;

        // Анализ только если кадр в градациях серого
        if (fb->format == PIXFORMAT_GRAYSCALE)
        {
          uint8_t *grayImage = fb->buf;

          // Анализ ROI области
          darkPixels = 0;
          totalPixels = settings.roi_width * settings.roi_height;

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

          darkRatio = (float)darkPixels / totalPixels;

          // Простая проверка условий
          if (darkRatio > settings.dark_min && darkRatio < settings.dark_max)
          {

            // здесь нужно задержаться и поизмерять в течении секунды -- расстояние - возможно авто заезжает в квадрат

            //
            //
            //
            //
            //

            if (darkPixels > settings.area && lastDistance != 0 && abs(lastDistance - settings.distance) > 50)
            {
              car_detected = true;
              Serial.printf("Car detected! Dark pixels: %d, ratio: %.2f, distance: %d\n", darkPixels, darkRatio, lastDistance);
            }
            else
            {
              Serial.printf("Car not detected! Dark pixels: %d, ratio: %.2f, distance: %d\n", darkPixels, darkRatio, lastDistance);
            }
          }
          else
          {
            Serial.printf("Car not detected 2! Dark pixels: %d, ratio: %.2f, distance: %d\n", darkPixels, darkRatio, lastDistance);
          }
        }

        esp_camera_fb_return(fb);

        // Если обнаружена машина - делаем качественный снимок
        if (car_detected)
        {
          Serial.println("Taking high quality photo...");

          onFlash();
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          offFlash();
          vTaskDelay(200 / portTICK_PERIOD_MS);

          sensor_t *s = esp_camera_sensor_get();

          s->set_framesize(s, FRAMESIZE_SVGA);
          s->set_pixformat(s, PIXFORMAT_JPEG);

          // Даем камере время на перестройку
          vTaskDelay(1500 / portTICK_PERIOD_MS);

          camera_fb_t *hi_res_fb = esp_camera_fb_get();

          if (hi_res_fb)
          {
            Serial.printf("Captured high-res frame: %zu bytes, format: %d\n", hi_res_fb->len, hi_res_fb->format);

            if (hi_res_fb->format == PIXFORMAT_JPEG && hi_res_fb->len > 0)
            {
              if (sd_initialized)
              {
                String num = (String)photoNumber;
                while (num.length() < 5)
                  num = "0" + num;

                // Генерируем имя файла с временной меткой
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
            esp_camera_fb_return(hi_res_fb);
          }
          else
          {
            Serial.println("High resolution camera capture failed");
          }

          vTaskDelay(250 / portTICK_PERIOD_MS);

          s->set_framesize(s, FRAMESIZE_QQVGA);
          s->set_pixformat(s, PIXFORMAT_GRAYSCALE);

          vTaskDelay(1500 / portTICK_PERIOD_MS);

          timeInterval = millis();
        }
      }
      else
      {
        Serial.println("Camera capture failed");
      }
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  server.handleClient();
  vTaskDelay(10 / portTICK_PERIOD_MS);
}