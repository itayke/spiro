#ifndef STORAGE_H
#define STORAGE_H

class Storage {
public:
  // Initialize NVS storage
  void init();

  // Load calibration values from NVS
  void loadCalibration(float& inhaleThreshold, float& exhaleThreshold);

  // Save calibration values to NVS
  void saveCalibration(float inhaleThreshold, float exhaleThreshold);
};

// Global storage instance (defined in main.cpp)
extern Storage storage;

#endif // STORAGE_H
