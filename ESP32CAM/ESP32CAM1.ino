#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char *ssid = "Caisse_CAM";
const char *password = "12345678";

// Liaison série matérielle 2 vers l'Arduino (GPIO 14 = RX, GPIO 15 = TX)
HardwareSerial ArduinoSerial(2);

// Serveur HTTP sur le port 80 pour faire le pont avec Python
WebServer server(80);

bool scanRequested = false;

void handleCheckScan() {
  if (scanRequested) {
    scanRequested = false;
    server.send(200, "text/plain", "SCAN");
  } else {
    server.send(200, "text/plain", "IDLE");
  }
}

void handleResultat() {
  if (server.hasArg("msg")) {
    String msg = server.arg("msg");
    ArduinoSerial.println(msg); // Transmet la réponse à l'Arduino via GPIO 15 -> A0
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "BAD PARAM");
  }
}

void handleFlash() {
  if (server.hasArg("state")) {
    int st = server.arg("state").toInt();
    digitalWrite(4, st ? HIGH : LOW);
  }
  server.send(200, "text/plain", "OK");
}

void startCameraServer();

void setup() {
  Serial.begin(115200);
  
  // Broche Flash (GPIO 4)
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  // Initialisation de la liaison vers l'Arduino
  ArduinoSerial.begin(9600, SERIAL_8N1, 14, 15);

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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_vflip(s, 1);
    s->set_hmirror(s, 0);
  }

  WiFi.softAP(ssid, password);

  // Configuration des routes du serveur HTTP
  server.on("/check_scan", handleCheckScan);
  server.on("/resultat", handleResultat);
  server.on("/flash", handleFlash);
  server.begin();

  startCameraServer(); // Stream vidéo sur port 81
}

void loop() {
  server.handleClient();

  // Écoute directe du signal 'S' envoyé par l'Arduino via le pont diviseur (GPIO 14)
  if (ArduinoSerial.available()) {
    String msg = ArduinoSerial.readStringUntil('\n');
    msg.trim();
    if (msg.indexOf("S") != -1) {
      // Déclenchement matériel instantané du flash
      digitalWrite(4, HIGH);
      scanRequested = true;
    }
  }
  delay(5);
}