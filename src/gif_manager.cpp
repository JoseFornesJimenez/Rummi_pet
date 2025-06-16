#include "gif_manager.h"

TFT_eSPI* GIFManager::_tft = nullptr;
uint16_t GIFManager::_lineBuffer[240];

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

  static bool opened = false;
  static unsigned long lastFrameTime = 0;
  static int frameDelay = 0;

  if (!opened) {
    if (_gif->open((uint8_t*)_currentGIF->data, _currentGIF->size, draw)) {
      _tft->startWrite();
      opened = true;
      frameDelay = 0;
      lastFrameTime = millis();
    } else {
      return;
    }
  }

  unsigned long now = millis();
  if (now - lastFrameTime >= frameDelay) {
    if (_gif->playFrame(true, &frameDelay)) {
      lastFrameTime = now;
    } else {
      _gif->close();
      _tft->endWrite();
      opened = false;
    }
  }
}

void GIFManager::draw(GIFDRAW *pDraw) {
  if (pDraw->y >= _tft->height()) return;

  int width = pDraw->iWidth;
  if (width + pDraw->iX > _tft->width()) width = _tft->width() - pDraw->iX;

  if (!pDraw->ucHasTransparency) {
    uint8_t *s = pDraw->pPixels;
    for (int i = 0; i < width; i++) {
      _lineBuffer[i] = pDraw->pPalette[*s++];
    }
    _tft->setAddrWindow(pDraw->iX, pDraw->iY + pDraw->y, width, 1);
    _tft->pushPixels(_lineBuffer, width);
  }
}
