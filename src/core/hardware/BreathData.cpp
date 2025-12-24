#include "BreathData.h"

#ifndef SIMULATOR
  #include <Arduino.h>
#else
  #include "Platform.h"
#endif

void BreathData::init() {
  currentState = BREATH_IDLE;
  breathStartTime = 0;
  lastBreathTime = 0;
  breathCount = 0;
  averageBreathDuration = 0;
  sessionStartTime = millis();
  inhaleThreshold = DEFAULT_INHALE_THRESHOLD;
  exhaleThreshold = DEFAULT_EXHALE_THRESHOLD;
  normalizedBreathRaw = 0;
  minPressureDelta = -10.0f;
  maxPressureDelta = 10.0f;
}

void BreathData::detect(float pressureDelta) {
  BreathState previousState = currentState;
  unsigned long now = millis();

  // Expand calibration bounds only when exceeding overage threshold
  // This filters out small noise spikes that would otherwise shrink normalized values
  if (pressureDelta < minPressureDelta * NORM_OVERAGE_THRESHOLD && minPressureDelta < -0.1f) {
    // Inhale exceeds threshold - expand gradually
    float ratio = pressureDelta / (minPressureDelta * NORM_OVERAGE_THRESHOLD);
    minPressureDelta *= ratio;
  }
  if (pressureDelta > maxPressureDelta * NORM_OVERAGE_THRESHOLD && maxPressureDelta > 0.1f) {
    // Exhale exceeds threshold - expand gradually
    float ratio = pressureDelta / (maxPressureDelta * NORM_OVERAGE_THRESHOLD);
    maxPressureDelta *= ratio;
  }

  // Calculate normalized breath (-1 to +1)
  if (pressureDelta < 0 && minPressureDelta < -0.1f) {
    // Inhale: map [minPressureDelta..0] → [-1..0]
    normalizedBreathRaw = pressureDelta / (-minPressureDelta);
  } else if (pressureDelta > 0 && maxPressureDelta > 0.1f) {
    // Exhale: map [0..maxPressureDelta] → [0..1]
    normalizedBreathRaw = pressureDelta / maxPressureDelta;
  } else {
    normalizedBreathRaw = 0;
  }

  // Detect breath state based on pressure delta
  if (pressureDelta < inhaleThreshold) {
    currentState = BREATH_INHALE;
  } else if (pressureDelta > exhaleThreshold) {
    currentState = BREATH_EXHALE;
  } else {
    // Check for breath hold (stable pressure for >3 seconds)
    if (now - lastBreathTime > BREATH_HOLD_TIMEOUT_MS &&
        abs(pressureDelta) < BREATH_HOLD_STABILITY_PA) {
      currentState = BREATH_HOLD;
    } else {
      currentState = BREATH_IDLE;
    }
  }

  // Detect breath transitions for counting
  if (previousState != currentState) {
    breathStartTime = now;

    // Count a full breath cycle when transitioning from exhale to inhale
    if (previousState == BREATH_EXHALE && currentState == BREATH_INHALE) {
      breathCount++;

      // Update average breath duration
      unsigned long cycleDuration = now - lastBreathTime;
      if (breathCount == 1) {
        averageBreathDuration = cycleDuration;
      } else {
        averageBreathDuration =
          (averageBreathDuration * (breathCount - 1) + cycleDuration) / breathCount;
      }
    }

    lastBreathTime = now;
  }
}

void BreathData::resetSession() {
  breathCount = 0;
  averageBreathDuration = 0;
  sessionStartTime = millis();
}

void BreathData::resetCalibration() {
  minPressureDelta = -10.0f;
  maxPressureDelta = 10.0f;
}
