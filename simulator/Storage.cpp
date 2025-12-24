// Simulator implementation of Storage (in-memory)
#include "core/hardware/Storage.h"
#include "config.h"

extern SerialMock Serial;

static float savedInhaleThreshold = DEFAULT_INHALE_THRESHOLD;
static float savedExhaleThreshold = DEFAULT_EXHALE_THRESHOLD;

void Storage::init() {
  Serial.println("Storage initialized (in-memory)");
}

void Storage::loadCalibration(float& inhaleThreshold, float& exhaleThreshold) {
  inhaleThreshold = savedInhaleThreshold;
  exhaleThreshold = savedExhaleThreshold;

  Serial.print("Loaded calibration - Inhale: ");
  Serial.print(inhaleThreshold);
  Serial.print(" Pa, Exhale: ");
  Serial.print(exhaleThreshold);
  Serial.println(" Pa");
}

void Storage::saveCalibration(float inhaleThreshold, float exhaleThreshold) {
  savedInhaleThreshold = inhaleThreshold;
  savedExhaleThreshold = exhaleThreshold;
  Serial.println("Calibration saved (in-memory)");
}
