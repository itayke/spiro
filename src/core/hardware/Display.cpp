#include "Display.h"
#include "config.h"

#ifndef SIMULATOR
  #include <Arduino.h>
#else
  #include "Platform.h"
#endif

void Display::init() {
  Serial.println("Initializing display...");

  _lcd.init();
  _lcd.setRotation(0);
  _lcd.fillScreen(TFT_BLACK);

  // Create sprite (canvas) for double-buffering
  _canvas.setColorDepth(16);
  _canvas.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);


  Serial.println("Display initialized");
}

LGFX& Display::getLcd() {
  return _lcd;
}

Canvas& Display::getCanvas() {
  return _canvas;
}

void Display::blit() {
  _canvas.pushSprite(&_lcd, 0, 0);

#ifdef SIMULATOR
  // Configure window on first blit
  static bool windowConfigured = false;
  if (!windowConfigured) {
    SDL_Window* window = SDL_GetWindowFromID(1);
    if (window) {
      // Bring window to front
      SDL_RaiseWindow(window);
      SDL_SetWindowInputFocus(window);
      // Make window non-resizable to maintain aspect ratio
      SDL_SetWindowResizable(window, SDL_FALSE);
      windowConfigured = true;
    }
  }
#endif
}

void Display::clear() {
  _lcd.fillScreen(TFT_BLACK);
}

void Display::showMessage(const char* message, uint16_t color) {
  _lcd.fillScreen(TFT_BLACK);
  _lcd.setCursor(10, SCREEN_HEIGHT / 2 - 10);
  _lcd.setTextColor(color);
  _lcd.setTextSize(1);
  _lcd.print(message);
}

uint16_t Display::rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
