#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// 🔥 YOUR WIFI
const char* ssid = ""; //change here
const char* password = ""; //change here

WebServer server(80);

// Trigger from DevKit
#define TRIGGER_PIN 13

// Flash LED
#define FLASH_LED 4

volatile bool triggerFlag = false;

// ================== INTERRUPT ==================
void IRAM_ATTR triggerISR() {
  triggerFlag = true;
}

// ================== STREAM ==================
void handleStream() {
  WiFiClient client = server.client();

  String header = "HTTP/1.1 200 OK\r\n";
  header += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(header);

  while (client.connected()) {

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) break;

    // 🔥 FLASH LED WHEN TRIGGERED
    if (triggerFlag) {
      digitalWrite(FLASH_LED, HIGH);
      delay(300);   // visible flash
      digitalWrite(FLASH_LED, LOW);
      triggerFlag = false;
    }

    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n\r\n");
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);

    delay(150); // stable stream
  }
}

// ================== ROOT ==================
void handleRoot() {
  String html = "<html><body>";
  html += "<h2>ESP32-CAM LIVE STREAM</h2>";
  html += "<p>Camera Ready</p>";
  html += "<img src='/stream'>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  // Disable brownout
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // LED setup
  pinMode(FLASH_LED, OUTPUT);
  digitalWrite(FLASH_LED, LOW);

  // Trigger pin setup
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), triggerISR, FALLING);

  // CAMERA CONFIG
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

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // 🔥 NO PSRAM SAFE SETTINGS
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 30;
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_DRAM;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  Serial.println("Camera OK");

  // ================== WIFI ==================
  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("Open this URL: http://");
  Serial.println(WiFi.localIP());

  // ================== SERVER ==================
  server.on("/", handleRoot);
  server.on("/stream", handleStream);

  server.begin();
}

// ================== LOOP ==================
void loop() {
  server.handleClient();
}