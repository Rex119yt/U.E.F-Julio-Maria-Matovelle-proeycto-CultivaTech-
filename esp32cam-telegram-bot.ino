#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "config.h"

// Pines
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FLASH_LED_PIN 4

// Variables globales
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

static camera_fb_t* fb_for_telegram = nullptr;
static size_t fb_pos = 0;

bool moreData() {
  return fb_pos < fb_for_telegram->len;
}

byte getNextByte() {
  return fb_for_telegram->buf[fb_pos++];
}

unsigned long lastTimeBotRan;
const unsigned long botInterval = 1000;


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

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
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error camara: 0x%x\n", err);
    return;
  }

  sensor_t* sensor = esp_camera_sensor_get();
  sensor->set_vflip(sensor, 1);

  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();
  bot.sendMessage(chatId, "ESP32-CAM iniciada y lista", "");
}


void loop() {
  if (millis() - lastTimeBotRan > botInterval) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {
        String msg_chat_id = bot.messages[i].chat_id;
        String text = bot.messages[i].text;

        if (msg_chat_id != String(chatId)) continue;

        if (text == "/foto") {
          captureAndSendPhoto();
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

// Capturar y enviar foto
void captureAndSendPhoto() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (fb) esp_camera_fb_return(fb);
  delay(50);

  digitalWrite(FLASH_LED_PIN, HIGH);
  delay(200);

  fb = esp_camera_fb_get();

  digitalWrite(FLASH_LED_PIN, LOW);

  if (!fb) {
    bot.sendMessage(chatId, "Error al capturar la foto", "");
    return;
  }

  fb_for_telegram = fb;
  fb_pos = 0;
  bool ok = bot.sendPhotoByBinary(chatId, "image/jpeg", fb->len, moreData, getNextByte, nullptr, nullptr);
  esp_camera_fb_return(fb);

  if (!ok) {
    bot.sendMessage(chatId, "Error al enviar la foto", "");
  }
}
