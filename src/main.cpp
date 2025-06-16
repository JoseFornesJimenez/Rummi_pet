#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include "gif_manager.h"
#include <Adafruit_NeoPixel.h>
#include "ftplayer.h"
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

void setup() {
  Serial.begin(115200); // Para depuración
  delay(1000); // Espera a que el puerto serie esté listo
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  gif.begin(BIG_ENDIAN_PIXELS);

  // Inicializa la tira de LEDs
  strip.begin();
  strip.show(); // Apaga todos los LEDs

  // Inicializa Serial1 en los pines correctos para ESP32C3 antes de usar DFPlayer
  Serial1.begin(9600, SERIAL_8N1, 20, 21); // RX=20, TX=21
  delay(100);
  // Inicializa el reproductor DFPlayer y muestra estado con LEDs
  bool dfplayerOk = ftplayer.begin(Serial1);
  delay(1000);
  if (dfplayerOk) {
    // Verde: DFPlayer conectado correctamente
    for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(0, 150, 0));
    strip.show();
    ftplayer.playSong(1);
    delay(1000);
  } else {
    // Rojo: Error de comunicación con DFPlayer
    for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(150, 0, 0));
    strip.show();
    while (1) delay(1000); // Detén el programa para depuración
  }

  // Apaga los LEDs antes de continuar
  strip.clear();
  strip.show();

  // Muestra el primer GIF
  gifManager.setGIF({ inicio, sizeof(inicio) });
  gifManager.play();
}

void loop() {
  unsigned long now = millis();

  // --- Animación LEDs sin delay y con velocidad constante ---
  static unsigned long lastLedUpdateLocal = 0;
  if (now - lastLedUpdateLocal > ledInterval) {
    lastLedUpdateLocal = now;
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
  static bool songPlaying = false;
  static int currentSong = 0;
  static unsigned long lastSongCheck = 0;

  // Detecta si la canción ha terminado (esto debe ir antes de intentar reproducir otra)
  if (songPlaying && (now - lastSongCheck > 300)) { // chequea cada 300ms
    lastSongCheck = now;
    if (ftplayer.isStopped()) {
      songPlaying = false;
    }
  }

  // Si no hay canción sonando, reinicializa el DFPlayer y reproduce aleatoriamente la 1 o la 2
  if (!songPlaying) {
    ftplayer.begin(Serial1); // Reinicializa DFPlayer
    currentSong = random(1, 3); // 1 o 2
    ftplayer.playSong(currentSong);
    songPlaying = true;
    lastSongCheck = now;
  }
  lastGifState = gifState; // Guarda el estado anterior del GIF
  while (gifState == lastGifState) { // Asegura que gifState cambie
    gifState = random(0, 4); // Selecciona un GIF aleatorio
  }
  if (now - lastGifUpdateLocal > gifInterval) {
    lastGifUpdateLocal = now;
    
    // Solo intenta reproducir si no hay canción en curso
    if (!songPlaying && random(5) == 0) {
      currentSong = random(1, 3); // 1 o 2
      ftplayer.playSong(currentSong);
      songPlaying = true;
      lastSongCheck = now; // reinicia el chequeo
    }
  }
  if(gifState == 0){
    //ftplayer.playSong(random(1,3)); // Reproduce la canción 1
    gifManager.setGIF(gifList[0]);
    gifManager.play();
    gifManager.play();
    lastGifState = 0;
  }
  if (gifState != lastGifState) {
    // tft.fillScreen(TFT_BLACK); // Quitado para evitar parpadeo
    gifManager.setGIF(gifList[gifState]);
    lastGifState = gifState;
  }
  gifManager.play();
  yield(); // Por si acaso
}