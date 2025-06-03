#include "gif_manager.h"

TFT_eSPI* GIFManager::_tft = nullptr;
uint16_t GIFManager::_lineBuffer[256];

GIFManager::GIFManager(TFT_eSPI* tft, AnimatedGIF* gif) {
  _tft = tft;
  _gif = gif;
  _currentGIF = nullptr;
}

void GIFManager::setGIF(const GIFEntry& gifEntry) {
  _currentGIF = &gifEntry;
}

void GIFManager::play() {
  if (!_currentGIF) return;
  static const GIFEntry* lastGif = nullptr;
  static bool isOpen = false;
  static unsigned long lastFrameTime = 0;
  static int frameDelay = 50; // valor por defecto en ms
  unsigned long now = millis();
  if (_currentGIF != lastGif || !isOpen) {
    if (isOpen) {
      _gif->close();
    }
    if (_gif->open((uint8_t*)_currentGIF->data, _currentGIF->size, draw)) {
      isOpen = true;
      lastGif = _currentGIF;
      lastFrameTime = now;
      frameDelay = 50;
    } else {
      isOpen = false;
      return;
    }
  }
  // Solo reproducir un frame si ha pasado el delay
  if ((long)(now - lastFrameTime) >= frameDelay) {
    int newDelay = 50;
    _tft->startWrite();
    if (!_gif->playFrame(true, &newDelay)) {
      _gif->reset();
    }
    _tft->endWrite();
    lastFrameTime = now;
    frameDelay = (newDelay > 0) ? newDelay : 50;
  }
}

void GIFManager::draw(GIFDRAW *pDraw) {
  // Centrado del GIF en pantalla
  int gifWidth = pDraw->iWidth;
  int gifHeight = pDraw->iHeight;
  int xOffset = (_tft->width() - gifWidth) / 2;
  int yOffset = (_tft->height() - gifHeight) / 2;

  if (pDraw->y + yOffset >= _tft->height()) return;

  int width = gifWidth;
  if (width + pDraw->iX + xOffset > _tft->width()) width = _tft->width() - (pDraw->iX + xOffset);

  if (!pDraw->ucHasTransparency) {
    uint8_t *s = pDraw->pPixels;
    for (int i = 0; i < width; i++) {
      _lineBuffer[i] = pDraw->pPalette[*s++];
    }
    _tft->setAddrWindow(pDraw->iX + xOffset, pDraw->iY + pDraw->y + yOffset, width, 1);
    _tft->pushPixels(_lineBuffer, width);
  }
}

