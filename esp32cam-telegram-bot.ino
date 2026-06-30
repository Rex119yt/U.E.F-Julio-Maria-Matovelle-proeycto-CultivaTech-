#include "esp_camera.h"

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

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);
  Serial.println();
  Serial.println("=== TEST DE CAMARA ESP32-CAM ===");

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

  Serial.print("Inicializando camara...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("ERROR");
    Serial.printf("Error codigo: 0x%x\n", err);
    return;
  }
  Serial.println("OK");

  sensor_t* sensor = esp_camera_sensor_get();
  sensor->set_vflip(sensor, 1);

  printSensorInfo(sensor);

  Serial.println("\nCapturando foto de prueba...");
  camera_fb_t* fb = esp_camera_fb_get();
  if (fb) {
    Serial.printf("Foto capturada: %dx%d, %d bytes\n", fb->width, fb->height, fb->len);
    Serial.printf("Formato: %s\n", fb->format == PIXFORMAT_JPEG ? "JPEG" : "Otro");
    esp_camera_fb_return(fb);
    Serial.println("Foto liberada correctamente");
  } else {
    Serial.println("ERROR: No se pudo capturar la foto");
  }

  Serial.println("\n=== PRUEBA FINALIZADA ===");
  Serial.println("La camara funciona correctamente");
}

void loop() {
  delay(10000);
}

void printSensorInfo(sensor_t* sensor) {
  Serial.println("\n--- Informacion del sensor ---");
  Serial.printf("Modelo: ");
  switch (sensor->id.PID) {
    case OV2640_PID: Serial.println("OV2640"); break;
    case OV3660_PID: Serial.println("OV3660"); break;
    case OV5640_PID: Serial.println("OV5640"); break;
    default: Serial.printf("0x%x\n", sensor->id.PID); break;
  }

  const char* sizes[] = {"96x96","QQVGA","QCIF","HQVGA","240x240","QVGA","CIF","HVGA",
                          "VGA","SVGA","XGA","HD","SXGA","UXGA","FHD","QXGA"};
  int idx = sensor->status.framesize;
  Serial.printf("Resolucion: %s (%d)\n", idx < 16 ? sizes[idx] : "?", idx);
  Serial.printf("Calidad JPEG: %d\n", sensor->status.quality);
  Serial.printf("Brillo: %d\n", sensor->status.brightness);
  Serial.printf("Contraste: %d\n", sensor->status.contrast);
  Serial.printf("Saturacion: %d\n", sensor->status.saturation);
  Serial.printf("AWB: %s\n", sensor->status.awb ? "ON" : "OFF");
  Serial.printf("AEC: %s\n", sensor->status.aec ? "ON" : "OFF");
  Serial.printf("AGC: %s\n", sensor->status.agc ? "ON" : "OFF");
  Serial.printf("V-Flip: %s\n", sensor->status.vflip ? "ON" : "OFF");
  Serial.printf("H-Mirror: %s\n", sensor->status.hmirror ? "ON" : "OFF");
  Serial.println("----------------------------");
}
