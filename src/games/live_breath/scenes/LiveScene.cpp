#include "LiveScene.h"
#include "config.h"
#include "core/hardware/BreathData.h"
#include "core/hardware/Display.h"

#ifndef SIMULATOR
  #include <Arduino.h>
#else
  #include "Platform.h"
#endif

LiveScene::LiveScene()
  : _wavePhase(0)
  , _targetWaveHeight(SCREEN_HEIGHT / 2)
  , _currentWaveHeight(SCREEN_HEIGHT / 2) {
}

void LiveScene::update(float dt) {
  // Animate wave phase (horizontal scroll)
  _wavePhase += 2.5f * dt;
  if (_wavePhase > TWO_PI) _wavePhase -= TWO_PI;

  // Calculate target wave height based on normalized breath (-1 to +1)
  float normalized = breathData.getNormalizedBreath();
  float maxDisplacement = 50.0f;
  _targetWaveHeight = (SCREEN_HEIGHT / 2) - (normalized * maxDisplacement);

  // Smooth interpolation for organic movement
  _currentWaveHeight += (_targetWaveHeight - _currentWaveHeight) * 0.1f;
}

void LiveScene::draw(Canvas& canvas) {
  // Fixed wave colors
  uint16_t waterColor = Display::rgb565(0, 120, 180);
  uint16_t foamColor = Display::rgb565(120, 180, 255);

  // Draw sky gradient
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    uint8_t brightness = map(y, 0, SCREEN_HEIGHT, 60, 20);
    uint16_t skyColor = Display::rgb565(brightness, brightness, brightness + 30);
    canvas.drawFastHLine(0, y, SCREEN_WIDTH, skyColor);
  }

  // Draw multi-layer wave for depth
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    float wave1 = sin(x * 0.15f + _wavePhase) * 8;
    float wave2 = sin(x * 0.08f + _wavePhase * 1.3f) * 5;
    float wave3 = sin(x * 0.22f - _wavePhase * 0.7f) * 3;

    int waveY = (int)(_currentWaveHeight + wave1 + wave2 + wave3);
    waveY = constrain(waveY, 10, SCREEN_HEIGHT - 10);

    // Draw foam/crest
    int foamHeight = abs((int)wave1) / 2 + 2;
    canvas.drawFastVLine(x, waveY - foamHeight, foamHeight, foamColor);

    // Draw water body below wave
    canvas.drawFastVLine(x, waveY, SCREEN_HEIGHT - waveY, waterColor);
  }

  // Draw HUD
  canvas.setCursor(4, 4);
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextSize(1);
  canvas.print("LIVE");

  // Breath count
  canvas.setCursor(4, SCREEN_HEIGHT - 10);
  canvas.print("Breaths: ");
  canvas.print(breathData.getBreathCount());

  // Breath state indicator (top right)
  const char* stateText;
  switch (breathData.getState()) {
    case BREATH_INHALE: stateText = "IN "; break;
    case BREATH_EXHALE: stateText = "OUT"; break;
    case BREATH_HOLD:   stateText = "HLD"; break;
    default:            stateText = "..."; break;
  }
  canvas.setCursor(SCREEN_WIDTH - 22, 4);
  canvas.print(stateText);
}
