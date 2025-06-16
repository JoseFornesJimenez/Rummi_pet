#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include "gif_manager.h"
#include <Adafruit_NeoPixel.h>
#include "ftplayer.h"
#include "ota_web.h"
#include "comm_manager.h"
#include <WiFi.h> // Se agrega para definir WL_CONNECTED y funciones WiFi
// GIFs
#include "gifs/inicio.h"
#include "gifs/normal.h"
#include <gifs/movimiento.h>
#include <gifs/hablar.h>
#include <gifs/mareado.h>
#define LED_PIN    9
#define NUM_LEDS   8
TFT_eSPI tft = TFT_eSPI();
AnimatedGIF gif;
GIFManager gifManager(&tft, &gif);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
FTPlayer ftplayer;

unsigned long lastLedUpdate = 0;
int ledIndex = 0;
bool forward = true;
const unsigned long ledInterval = 10; // ms

// --- Lista de GIFs disponibles ---
GIFEntry gifList[] = {
  { mareado, sizeof(mareado) },
  { hablar, sizeof(hablar) },
  { normal, sizeof(normal) },
  { movimiento, sizeof(movimiento) },
  { inicio, sizeof(inicio) }
};
const int numGifs = sizeof(gifList) / sizeof(gifList[0]);

unsigned long lastGifUpdate = 0;
int gifState = 0; // índice del GIF actual
int lastGifState = 0;
const unsigned long gifInterval = 1000; // ms

void showWiFiStatusOnScreen() {
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 30);
  if (WiFi.status() == WL_CONNECTED) {
    tft.println("WiFi: Conectado");
    tft.setCursor(10, 60);
    tft.print("IP: ");
    tft.println(WiFi.localIP());
  } else {
    tft.println("WiFi: NO conectado");
    tft.setCursor(10, 60);
    tft.print("IP: ");
    tft.println("-");
  }
  delay(2500); // Muestra la info 2.5 segundos
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("[AP] Iniciando modo AP...");
  WiFi.disconnect(true, true);
  delay(200);
  WiFi.mode(WIFI_AP);
  delay(200);
  bool apResult = WiFi.softAP("ESP32C3_AP", NULL, 1); // Sin clave, canal 1
  delay(500);
  Serial.print("[AP] Resultado WiFi.softAP: ");
  Serial.println(apResult ? "OK" : "FALLO");
  Serial.print("[AP] IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // No hace nada
}