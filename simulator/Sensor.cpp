// Simulator implementation of Sensor
#include "core/hardware/Sensor.h"
#include "core/hardware/Display.h"
#include "Platform.h"
#include "config.h"

extern SerialMock Serial;

void Sensor::init() {
  Serial.println("Initializing simulated sensor...");
  Serial.println("Use mouse Y position (screen-relative) to simulate breath pressure");
  Serial.println("  - Move mouse UP = Exhale (positive pressure)");
  Serial.println("  - Move mouse DOWN = Inhale (negative pressure)");
  Serial.println("  - Screen center = Neutral");
  Serial.println("Simulated sensor initialized!");

  baselinePressure = 101325.0f;  // Standard atmospheric pressure (Pa)
  currentPressure = 101325.0f;
  currentTemperature = 22.0f;    // Room temperature (C)
}

void Sensor::calibrateBaseline() {
  Serial.println("Calibrating baseline (simulated)...");
  display.showMessage("Calibrating...\n  Move mouse\n  to center", TFT_CYAN);
  delay(1000);
  display.clear();
  Serial.println("Baseline calibrated (simulated)");
}

void Sensor::setMouseY(int mouseY, int windowHeight) {
  _mouseY = mouseY;
  _windowHeight = windowHeight;
}

void Sensor::update() {
  // Get global mouse position (absolute screen coordinates)
  int globalMouseY;
  SDL_GetGlobalMouseState(nullptr, &globalMouseY);

  // Get display bounds to find screen center
  SDL_DisplayMode displayMode;
  SDL_GetCurrentDisplayMode(0, &displayMode);
  int screenCenterY = displayMode.h / 2;

  // Map global mouse Y to pressure delta, normalized to screen center
  // Screen center = 0 Pa
  // Top of screen = +50 Pa (exhale)
  // Bottom of screen = -50 Pa (inhale)
  float normalizedY = (float)(globalMouseY - screenCenterY) / (float)screenCenterY;  // -1 to +1
  normalizedY = std::max(-1.0f, std::min(1.0f, normalizedY));  // Clamp

  // Scale to pressure range (±50 Pa is typical breath range)
  // Negate so up = exhale (positive), down = inhale (negative)
  pressureDelta = -normalizedY * 50.0f;

  // Update absolute pressure for display
  currentPressure = baselinePressure + pressureDelta;
}
