#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include "gif_manager.h"
#include <Adafruit_NeoPixel.h>
// GIFs
#include "gifs/guino.h"
#include "gifs/estrella.h"  // Este será el GIF "mareado"
#include "gifs/inicio.h"
#include "gifs/normal.h"
#include "gifs/mareado.h"
#include "gifs/corazones.h"
#include <gifs/movimiento1.h>
#include <gifs/muerto.h>
#include <gifs/baba.h>

#define LED_PIN    9
#define NUM_LEDS   8
TFT_eSPI tft = TFT_eSPI();
AnimatedGIF gif;
GIFManager gifManager(&tft, &gif);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastLedUpdate = 0;
int ledIndex = 0;
bool forward = true;
const unsigned long ledInterval = 10; // ms

// --- Lista de GIFs disponibles ---
GIFEntry gifList[] = {
  { normal, sizeof(normal) },
  { estrella, sizeof(estrella) },
  { guino, sizeof(guino) },
  { mareado, sizeof(mareado) },
  { corazones, sizeof(corazones) },
  { movimiento1, sizeof(movimiento1) },
  { muerto, sizeof(muerto) },
  { baba, sizeof(baba) }
};
const int numGifs = sizeof(gifList) / sizeof(gifList[0]);

unsigned long lastGifUpdate = 0;
int gifState = 0; // índice del GIF actual
const unsigned long gifInterval = 1000; // ms

void setup() {
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  gif.begin(BIG_ENDIAN_PIXELS);

  // Inicializa la tira de LEDs
  strip.begin();
  strip.show(); // Apaga todos los LEDs

  // Muestra el primer GIF
  gifManager.setGIF({ normal, sizeof(normal) });
  gifManager.play();
}

void loop() {
  unsigned long now = millis();

  // --- Animación LEDs sin delay ---
  if (now - lastLedUpdate > ledInterval) {
    lastLedUpdate = now;
    strip.clear();
    if (forward) {
      strip.setPixelColor(ledIndex, strip.Color(150, 0, 0)); // Verde
      ledIndex++;
      if (ledIndex >= NUM_LEDS) {
        ledIndex = NUM_LEDS - 2;
        forward = false;
      }
    } else {
      strip.setPixelColor(ledIndex, strip.Color(150, 0, 0)); // Azul
      ledIndex--;
      if (ledIndex < 0) {
        ledIndex = 1;
        forward = true;
      }
    }
    strip.show();
    yield(); // Añadido para evitar WDT
  }

  // --- Animación GIFs sin delay y sin parpadeo ---
  static int lastGifState = -1;
  static unsigned long lastGifUpdateLocal = 0;
  if (now - lastGifUpdateLocal > gifInterval) {
    lastGifUpdateLocal = now;
    gifState = (gifState + 1) % numGifs;
  }
  if (gifState != lastGifState) {
    gifManager.setGIF(gifList[gifState]);
    lastGifState = gifState;
  }
  gifManager.play();
  yield(); // Por si acaso
}