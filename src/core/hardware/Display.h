#ifndef DISPLAY_H
#define DISPLAY_H

#include "LGFX_Config.hpp"

// Color definitions (LovyanGFX compatible)
#define TFT_BLACK       0x0000
#define TFT_WHITE       0xFFFF
#define TFT_RED         0xF800
#define TFT_GREEN       0x07E0
#define TFT_BLUE        0x001F
#define TFT_CYAN        0x07FF
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_ORANGE      0xFD20
#define TFT_GRAY        0x8410

class Display {
public:
  // Initialize display
  void init();

  // Get reference to canvas for double-buffered drawing
  Canvas& getCanvas();

  // Get reference to LCD for direct access
  LGFX& getLcd();

  // Blit canvas to display
  void blit();

  // Clear screen to black
  void clear();

  // Show a message centered on screen
  void showMessage(const char* message, uint16_t color);

  // Convert RGB to 565 format
  static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);

private:
  LGFX _lcd;
  Canvas _canvas;
};

// Global display instance (defined in main.cpp)
extern Display display;

#endif // DISPLAY_H
