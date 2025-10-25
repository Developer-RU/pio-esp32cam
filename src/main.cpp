// // // // // #include <Arduino.h>
// // // // // #include "esp_camera.h"
// // // // // #include <WiFi.h>

// // // // // #define CAMERA_MODEL_AI_THINKER

// // // // // #include "camera_pins.h"

// // // // // const char *ssid = "Keenetic-8947";
// // // // // const char* password = "3Y5awJSE";

// // // // // void startCameraServer();

// // // // // void setup() {
// // // // //   Serial.begin(115200);
// // // // //   Serial.setDebugOutput(true);
// // // // //   Serial.println();

// // // // //   camera_config_t config;
// // // // //   config.ledc_channel = LEDC_CHANNEL_0;
// // // // //   config.ledc_timer = LEDC_TIMER_0;
// // // // //   config.pin_d0 = Y2_GPIO_NUM;
// // // // //   config.pin_d1 = Y3_GPIO_NUM;
// // // // //   config.pin_d2 = Y4_GPIO_NUM;
// // // // //   config.pin_d3 = Y5_GPIO_NUM;
// // // // //   config.pin_d4 = Y6_GPIO_NUM;
// // // // //   config.pin_d5 = Y7_GPIO_NUM;
// // // // //   config.pin_d6 = Y8_GPIO_NUM;
// // // // //   config.pin_d7 = Y9_GPIO_NUM;
// // // // //   config.pin_xclk = XCLK_GPIO_NUM;
// // // // //   config.pin_pclk = PCLK_GPIO_NUM;
// // // // //   config.pin_vsync = VSYNC_GPIO_NUM;
// // // // //   config.pin_href = HREF_GPIO_NUM;
// // // // //   config.pin_sscb_sda = SIOD_GPIO_NUM;
// // // // //   config.pin_sscb_scl = SIOC_GPIO_NUM;
// // // // //   config.pin_pwdn = PWDN_GPIO_NUM;
// // // // //   config.pin_reset = RESET_GPIO_NUM;
// // // // //   config.xclk_freq_hz = 20000000;
// // // // //   config.pixel_format = PIXFORMAT_JPEG;

// // // // //   // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
// // // // //   //                      for larger pre-allocated frame buffer.
// // // // //   if(psramFound()){
// // // // //     config.frame_size = FRAMESIZE_UXGA;
// // // // //     config.jpeg_quality = 10;
// // // // //     config.fb_count = 2;
// // // // //   } else {
// // // // //     config.frame_size = FRAMESIZE_SVGA;
// // // // //     config.jpeg_quality = 12;
// // // // //     config.fb_count = 1;
// // // // //   }

// // // // // #if defined(CAMERA_MODEL_ESP_EYE)
// // // // //   pinMode(13, INPUT_PULLUP);
// // // // //   pinMode(14, INPUT_PULLUP);
// // // // // #endif

// // // // //   // camera init
// // // // //   esp_err_t err = esp_camera_init(&config);
// // // // //   if (err != ESP_OK) {
// // // // //     Serial.printf("Camera init failed with error 0x%x", err);
// // // // //     return;
// // // // //   }

// // // // //   sensor_t * s = esp_camera_sensor_get();
// // // // //   // initial sensors are flipped vertically and colors are a bit saturated
// // // // //   if (s->id.PID == OV3660_PID) {
// // // // //     s->set_vflip(s, 1); // flip it back
// // // // //     s->set_brightness(s, 1); // up the brightness just a bit
// // // // //     s->set_saturation(s, -2); // lower the saturation
// // // // //   }
// // // // //   // drop down frame size for higher initial frame rate
// // // // //   s->set_framesize(s, FRAMESIZE_QVGA);

// // // // // #if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
// // // // //   s->set_vflip(s, 1);
// // // // //   s->set_hmirror(s, 1);
// // // // // #endif

// // // // //   WiFi.begin(ssid, password);

// // // // //   while (WiFi.status() != WL_CONNECTED) {
// // // // //     delay(500);
// // // // //     Serial.print(".");
// // // // //   }
// // // // //   Serial.println("");
// // // // //   Serial.println("WiFi connected");

// // // // //   startCameraServer();

// // // // //   Serial.print("Camera Ready! Use 'http://");
// // // // //   Serial.print(WiFi.localIP());
// // // // //   Serial.println("' to connect");
// // // // // }

// // // // // void loop() {
// // // // //   // put your main code here, to run repeatedly:
// // // // //   delay(10000);
// // // // // }

















#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>

#define FLASH_GPIO_NUM 4

// ======= Настройки камеры (ESP32-CAM AI Thinker) =======
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

// ======= Настройки WiFi (опционально, если нужно отладка по сети) =======
// // // // // const char *ssid = "Keenetic-8947";
// // // // // const char *password = "3Y5awJSE";

const char *ssid = "CAR-DETECTOR";
const char *password = "";



// ======= Порог и зона анализа =======
const int REGION_WIDTH = 100;           // ширина зоны анализа в центре
const int REGION_HEIGHT = 80;           // высота зоны анализа
const float DETECTION_THRESHOLD = 15.0; // порог изменения яркости

float lastAverage = 0;

void startCameraServer();

void setup()
{
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  digitalWrite(FLASH_GPIO_NUM, LOW);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Инициализация камеры
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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_QQVGA; // 160x120
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Ошибка инициализации камеры 0x%x", err);
    return;
  }


  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());


  // // // // // // Подключение к WiFi (если нужно)
  // // // // // WiFi.begin(ssid, password);
  // // // // // Serial.print("Подключение к WiFi");
  // // // // // while (WiFi.status() != WL_CONNECTED)
  // // // // // {
  // // // // //   delay(500);
  // // // // //   Serial.print(".");
  // // // // // }

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop()
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Ошибка получения кадра");
    delay(1000);
    return;
  }

  // Размер кадра
  int width = 160;
  int height = 120;

  // Центрируем прямоугольник
  int x0 = (width - REGION_WIDTH) / 2;
  int y0 = (height - REGION_HEIGHT) / 2;

  // Подсчёт среднего серого в центре
  unsigned long sum = 0;
  int count = 0;

  for (int y = y0; y < y0 + REGION_HEIGHT; y++)
  {
    for (int x = x0; x < x0 + REGION_WIDTH; x++)
    {
      int i = y * width + x;
      sum += fb->buf[i];
      count++;
    }
  }

  float avgGray = (float)sum / count;
  float diff = abs(avgGray - lastAverage);

  if (lastAverage != 0 && diff > DETECTION_THRESHOLD)
  {
    Serial.printf("🚗 Автомобиль (или объект) обнаружен! Изменение = %.2f\n", diff);

    digitalWrite(FLASH_GPIO_NUM, HIGH);
    delay(5000);
    digitalWrite(FLASH_GPIO_NUM, LOW);
    delay(5000);
  }
  else
  {
    Serial.printf("Нет авто. Текущее изменение = %.2f\n", diff);
  }

  lastAverage = avgGray;

  esp_camera_fb_return(fb);
  delay(1000); // Снимать 1 раз в секунду
}
