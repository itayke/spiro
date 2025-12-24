#ifndef CONFIG_H
#define CONFIG_H

#ifndef SIMULATOR
  #include <Arduino.h>
#else
  #include "Platform.h"
#endif

// ========================================
// Pin Definitions
// ========================================
// ST7735S Display (SPI)
#define TFT_CS    15
#define TFT_RST   4
#define TFT_DC    2
#define TFT_MOSI  23
#define TFT_SCLK  18

// BMP280 Sensor (I2C)
#define BMP_SDA   21
#define BMP_SCL   22

// ========================================
// Display Configuration
// ========================================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// Custom color definitions (not in all ST7735 library versions)
#define ST77XX_GRAY   0x8410  // RGB(128, 128, 128)

// ========================================
// Application Modes
// ========================================
enum AppMode {
  MODE_LIVE,
  MODE_DIAGNOSTIC,
  MODE_BALLOON
};

// ========================================
// Breath Detection
// ========================================
enum BreathState {
  BREATH_IDLE,
  BREATH_INHALE,
  BREATH_EXHALE,
  BREATH_HOLD
};

// Default calibration thresholds (Pa)
#define DEFAULT_INHALE_THRESHOLD  -5.0f
#define DEFAULT_EXHALE_THRESHOLD   5.0f
#define BREATH_HOLD_TIMEOUT_MS     3000
#define BREATH_HOLD_STABILITY_PA   2.0f

// Normalization overage threshold (1.1 = 10% beyond bounds before expanding)
#define NORM_OVERAGE_THRESHOLD     1.25f

// ========================================
// Update Rates
// ========================================
#define MAIN_LOOP_DELAY_MS        20    // ~50Hz
#define WAVE_UPDATE_FPS           30
#define DIAGNOSTIC_UPDATE_FPS     10

#endif // CONFIG_H
