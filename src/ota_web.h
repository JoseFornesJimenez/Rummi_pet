#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
extern TFT_eSPI tft;
void setupOTA();
void handleWebServer();
void drawWiFiStatusIcon();
