#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#ifdef SIMULATOR
  #include <lgfx/v1/platforms/sdl/Panel_sdl.hpp>
#endif

#include "config.h"

class LGFX : public lgfx::LGFX_Device {
public:
  LGFX() {
#ifdef SIMULATOR
    // SDL2 panel for desktop simulator
    auto panel = new lgfx::Panel_sdl();
    auto cfg = panel->config();
    cfg.memory_width  = SCREEN_WIDTH;
    cfg.memory_height = SCREEN_HEIGHT;
    cfg.panel_width   = SCREEN_WIDTH;
    cfg.panel_height  = SCREEN_HEIGHT;
    panel->config(cfg);
    panel->setScaling(4, 4);  // 4x scale for better visibility
    panel->setWindowTitle("Inhale Simulator");
    setPanel(panel);
#else
    // ST7735S panel for ESP32 hardware
    auto bus = new lgfx::Bus_SPI();
    {
      auto cfg = bus->config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 27000000;
      cfg.freq_read  = 16000000;
      cfg.pin_sclk = TFT_SCLK;
      cfg.pin_mosi = TFT_MOSI;
      cfg.pin_miso = -1;
      cfg.pin_dc   = TFT_DC;
      bus->config(cfg);
    }

    auto panel = new lgfx::Panel_ST7735S();
    {
      auto cfg = panel->config();
      cfg.pin_cs   = TFT_CS;
      cfg.pin_rst  = TFT_RST;
      cfg.pin_busy = -1;
      cfg.memory_width  = SCREEN_WIDTH;
      cfg.memory_height = SCREEN_HEIGHT;
      cfg.panel_width   = SCREEN_WIDTH;
      cfg.panel_height  = SCREEN_HEIGHT;
      cfg.offset_x = 2;
      cfg.offset_y = 1;
      cfg.offset_rotation = 0;
      cfg.rgb_order = false;
      cfg.invert = false;
      panel->config(cfg);
    }

    panel->setBus(bus);
    setPanel(panel);
#endif
  }
};

// Global display instance type
using Canvas = LGFX_Sprite;
