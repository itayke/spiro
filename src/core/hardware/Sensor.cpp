#include "Sensor.h"
#include "Display.h"
#include "config.h"
#include <Wire.h>

#ifdef USE_BME280
  #include <Adafruit_BME280.h>
  static Adafruit_BME280 bme;
#elif defined(USE_BMP280)
  #include <Adafruit_BMP280.h>
  static Adafruit_BMP280 bmp;
#else
  #error "Must define either USE_BME280 or USE_BMP280"
#endif

void Sensor::init() {
#ifdef USE_BME280
  Serial.println("Initializing BME280 sensor...");
  delay(100);

  unsigned status = bme.begin(0x76);
  if (!status) {
    Serial.println("ERROR: Could not find BME280 sensor at 0x76!");
    Serial.print("SensorID was: 0x");
    Serial.println(bme.sensorID(), HEX);

    Serial.println("Trying alternate address 0x77...");
    status = bme.begin(0x77);
    if (!status) {
      Serial.println("Failed at 0x77 too!");
      Serial.println("ID of 0xFF = bad address or BMP180/BMP085");
      Serial.println("ID of 0x56-0x58 = BMP280");
      Serial.println("ID of 0x60 = BME280");
      while (1) delay(100);
    }
  }

  Serial.println("BME280 initialized successfully!");

  // Configure BME280 for high precision
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X16,  // Pressure oversampling
                  Adafruit_BME280::SAMPLING_X2,   // Temperature oversampling
                  Adafruit_BME280::SAMPLING_NONE, // Humidity (not needed)
                  Adafruit_BME280::FILTER_X16,    // Filtering
                  Adafruit_BME280::STANDBY_MS_0_5); // Standby time

#elif defined(USE_BMP280)
  Serial.println("Initializing BMP280 sensor...");
  delay(100);

  // Initialize BMP280 - specify chip ID explicitly for GY-BMP280 clones
  unsigned status = bmp.begin(0x76, 0x58);
  if (!status) {
    Serial.println("ERROR: Could not find BMP280 sensor at 0x76!");
    Serial.print("SensorID was: 0x");
    Serial.println(bmp.sensorID(), HEX);

    Serial.println("Trying alternate address 0x77 with chip ID 0x58...");
    status = bmp.begin(0x77, 0x58);
    if (!status) {
      Serial.println("Failed at 0x77 too!");
      Serial.println("ID of 0xFF = bad address or BMP180/BMP085");
      Serial.println("ID of 0x56-0x58 = BMP280");
      Serial.println("ID of 0x60 = BME280");
      while (1) delay(100);
    }
  }

  Serial.println("BMP280 initialized successfully!");

  // Configure BMP280 for high precision
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X16,  // Pressure oversampling
                  Adafruit_BMP280::SAMPLING_X2,   // Temperature oversampling
                  Adafruit_BMP280::FILTER_X16,    // Filtering
                  Adafruit_BMP280::STANDBY_MS_1); // Standby time
#endif
}

void Sensor::calibrateBaseline() {
  Serial.println("Calibrating baseline pressure...");

  display.showMessage("Calibrating...\n  Breathe\n  normally", TFT_CYAN);

  // Take average of 50 readings
  float sum = 0;
  for (int i = 0; i < 50; i++) {
#ifdef USE_BME280
    sum += bme.readPressure();
#else
    sum += bmp.readPressure();
#endif
    delay(20);
  }

  baselinePressure = sum / 50.0f;

  Serial.print("Baseline pressure: ");
  Serial.print(baselinePressure);
  Serial.println(" Pa");

  display.clear();
}

void Sensor::update() {
#ifdef USE_BME280
  currentPressure = bme.readPressure();
  currentTemperature = bme.readTemperature();
#else
  currentPressure = bmp.readPressure();
  currentTemperature = bmp.readTemperature();
#endif
  pressureDelta = currentPressure - baselinePressure;
}
