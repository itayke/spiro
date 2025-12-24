#ifdef SIMULATOR
  #include "Platform.h"
#else
  #include <Arduino.h>
#endif

#include "config.h"
#include "core/hardware/BreathData.h"
#include "core/hardware/Display.h"
#include "core/hardware/Sensor.h"
#include "core/hardware/Storage.h"
#include "scenes/LiveScene.h"

// ========================================
// Global Application State
// ========================================
#ifdef SIMULATOR
SerialMock Serial;
#endif

BreathData breathData;
Display display;
Sensor pressureSensor;
Storage storage;
LiveScene* liveScene = nullptr;

// Scene frame timing
unsigned long lastSceneUpdate = 0;

// ========================================
// Setup
// ========================================
void setup() {
  Serial.begin(115200);

#ifdef SIMULATOR
  Serial.println("Spiro - Live Breath Visualization");
  Serial.println("==================================");
  Serial.println("Controls:");
  Serial.println("  Mouse Y: Breath pressure (up=exhale, down=inhale)");
  Serial.println("  ESC/Q: Quit");
  Serial.println("");
#else
  delay(1000);
  Serial.println("Spiro - Live Breath Visualization");
  Serial.println("==================================");
#endif

  // Initialize components
  display.init();
  pressureSensor.init();
  storage.init();
  breathData.init();

  // Load calibration from storage
  storage.loadCalibration(breathData.inhaleThreshold, breathData.exhaleThreshold);

  // Calibrate baseline
  pressureSensor.calibrateBaseline();

  // Create live scene
  liveScene = new LiveScene();
  liveScene->init();
  lastSceneUpdate = millis();

  Serial.println("Live breath game ready!");
}

// ========================================
// Main Loop
// ========================================
void loop() {
  // Update sensor readings
  pressureSensor.update();
  float pressureDelta = pressureSensor.getDelta();

  // Detect breath state
  breathData.detect(pressureDelta);

  // Update and draw scene at target FPS
  if (liveScene) {
    unsigned long now = millis();
    unsigned long frameInterval = 1000 / liveScene->getFps();

    if (now - lastSceneUpdate >= frameInterval) {
      float dt = (now - lastSceneUpdate) / 1000.0f;
      lastSceneUpdate = now;

      liveScene->update(dt);

      Canvas& canvas = display.getCanvas();
      liveScene->draw(canvas);
      display.blit();
    }
  }

#ifndef SIMULATOR
  delay(MAIN_LOOP_DELAY_MS);
#endif
}

// ========================================
// Simulator Entry Point
// ========================================
#ifdef SIMULATOR
#include <lgfx/v1/platforms/sdl/Panel_sdl.hpp>

static bool handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        return false;

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
          case SDLK_q:
            return false;
        }
        break;

      case SDL_MOUSEMOTION:
        pressureSensor.setMouseY(event.motion.y, SCREEN_HEIGHT * 4);
        break;
    }
  }
  return true;
}

int main(int argc, char* argv[]) {
  // Initialize Panel_sdl (this also calls SDL_Init internally)
  if (lgfx::Panel_sdl::setup() != 0) {
    Serial.println("Panel_sdl::setup failed!");
    return 1;
  }

  setup();

  uint32_t lastLoopTime = 0;

  // Main loop - Panel_sdl::loop() returns non-zero when window is closed
  while (lgfx::Panel_sdl::loop() == 0) {
    if (!handleEvents()) {
      break;
    }

    // Run main loop at ~50Hz
    uint32_t now = millis();
    if (now - lastLoopTime >= MAIN_LOOP_DELAY_MS) {
      lastLoopTime = now;
      loop();
    }
  }

  lgfx::Panel_sdl::close();
  return 0;
}
#endif
