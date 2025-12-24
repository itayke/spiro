#ifndef BREATH_DATA_H
#define BREATH_DATA_H

#include "config.h"

class BreathData {
public:
  // Initialize breath detection
  void init();

  // Update breath detection based on current pressure
  void detect(float pressureDelta);

  // Reset session statistics
  void resetSession();

  // Reset min/max calibration bounds
  void resetCalibration();

  // Getters
  BreathState getState() const { return currentState; }
  int getBreathCount() const { return breathCount; }
  float getAverageBreathDuration() const { return averageBreathDuration; }
  unsigned long getSessionStartTime() const { return sessionStartTime; }
  unsigned long getBreathStartTime() const { return breathStartTime; }

  // Normalized breath: -1 (max inhale) to +1 (max exhale)
  float getNormalizedBreath() const { return constrain(normalizedBreathRaw, -1.0f, 1.0f); }
  // Raw normalized breath value (may exceed -1 to +1)
  float getNormalizedBreathRaw() const { return normalizedBreathRaw; }

  // Calibration bounds (for diagnostics)
  float getMinDelta() const { return minPressureDelta; }
  float getMaxDelta() const { return maxPressureDelta; }

  // Calibration thresholds
  float inhaleThreshold;
  float exhaleThreshold;

private:
  BreathState currentState = BREATH_IDLE;
  unsigned long breathStartTime = 0;
  unsigned long lastBreathTime = 0;
  int breathCount = 0;
  float averageBreathDuration = 0;
  unsigned long sessionStartTime = 0;

  // Normalization
  float normalizedBreathRaw = 0;
  float minPressureDelta = -10.0f;  // Initial estimate (inhale)
  float maxPressureDelta = 10.0f;   // Initial estimate (exhale)
};

// Global breath data instance (defined in main.cpp)
extern BreathData breathData;

#endif // BREATH_DATA_H
