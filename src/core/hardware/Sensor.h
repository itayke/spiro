#ifndef SENSOR_H
#define SENSOR_H

class Sensor {
public:
  // Initialize sensor
  void init();

  // Calibrate baseline pressure (call once at startup)
  void calibrateBaseline();

  // Update current pressure reading (call every loop)
  void update();

  // Get raw pressure delta from baseline (in Pascals)
  float getDelta() const { return pressureDelta; }

  // Get absolute pressure in Pascals
  float getAbsolutePressure() const { return currentPressure; }

  // Get current temperature in Celsius
  float getTemperature() const { return currentTemperature; }

#ifdef SIMULATOR
  // Simulator only: set pressure from mouse Y position
  void setMouseY(int mouseY, int windowHeight);
private:
  int _mouseY = 256;      // Start at center (neutral)
  int _windowHeight = 512;
#endif

private:
  float baselinePressure = 0;
  float currentPressure = 0;
  float currentTemperature = 0;
  float pressureDelta = 0;
};

// Global sensor instance (defined in main.cpp)
extern Sensor pressureSensor;

#endif // SENSOR_H
