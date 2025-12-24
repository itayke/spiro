#include "DiagnosticScene.h"
#include "config.h"
#include "core/hardware/BreathData.h"
#include "core/hardware/Display.h"
#include "core/hardware/Sensor.h"

#ifndef SIMULATOR
  #include <Arduino.h>
#else
  #include "Platform.h"
#endif

DiagnosticScene::DiagnosticScene()
  : _pressureDelta(0) {
}

void DiagnosticScene::update(float dt) {
  _pressureDelta = pressureSensor.getDelta();
}

void DiagnosticScene::draw(Canvas& canvas) {
  canvas.fillScreen(TFT_BLACK);

  // Title
  canvas.setCursor(10, 5);
  canvas.setTextColor(TFT_YELLOW);
  canvas.setTextSize(1);
  canvas.println("DIAGNOSTIC MODE");

  // Pressure delta
  canvas.setCursor(10, 22);
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextSize(1);
  canvas.print("Delta: ");
  if (_pressureDelta >= 0) {
    canvas.setTextColor(TFT_CYAN);
    canvas.print("+");
  } else {
    canvas.setTextColor(TFT_MAGENTA);
  }
  canvas.print(_pressureDelta, 2);
  canvas.print(" Pa");

  // Normalized value
  float normalized = breathData.getNormalizedBreath();
  canvas.setCursor(10, 36);
  canvas.setTextColor(TFT_WHITE);
  canvas.print("Norm: ");
  canvas.setTextColor(normalized >= 0 ? TFT_CYAN : TFT_MAGENTA);
  canvas.print(normalized, 2);

  // Draw normalized bar (-1 to +1)
  int barY = 54;
  int barCenter = SCREEN_WIDTH / 2;
  int maxBarWidth = (SCREEN_WIDTH - 20) / 2;  // Half width for each direction
  int barWidth = abs(normalized) * maxBarWidth;

  // Check if bounds are being pushed (exceeds overage threshold)
  float minDelta = breathData.getMinDelta();
  float maxDelta = breathData.getMaxDelta();
  bool pushingMin = _pressureDelta < minDelta * NORM_OVERAGE_THRESHOLD && minDelta < -0.1f;
  bool pushingMax = _pressureDelta > maxDelta * NORM_OVERAGE_THRESHOLD && maxDelta > 0.1f;

  canvas.drawFastHLine(10, barY, SCREEN_WIDTH - 20, TFT_GRAY);
  canvas.drawFastVLine(barCenter, barY - 5, 10, TFT_WHITE);

  if (normalized > 0) {
    canvas.fillRect(barCenter, barY - 3, barWidth, 6, TFT_CYAN);
    // Draw arrow if pushing max
    if (pushingMax) {
      int arrowX = barCenter + barWidth;
      canvas.fillTriangle(arrowX, barY - 5, arrowX, barY + 5, arrowX + 6, barY, TFT_WHITE);
    }
  } else {
    canvas.fillRect(barCenter - barWidth, barY - 3, barWidth, 6, TFT_MAGENTA);
    // Draw arrow if pushing min
    if (pushingMin) {
      int arrowX = barCenter - barWidth;
      canvas.fillTriangle(arrowX, barY - 5, arrowX, barY + 5, arrowX - 6, barY, TFT_WHITE);
    }
  }

  // Absolute pressure in inHg
  canvas.setCursor(10, 68);
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextSize(1);
  canvas.print("Pressure:");

  canvas.setCursor(10, 80);
  canvas.setTextSize(2);
  canvas.setTextColor(TFT_GREEN);
  float pressureInHg = pressureSensor.getAbsolutePressure() / 3386.39;  // Pa to inHg
  canvas.print(pressureInHg, 3);
  canvas.setTextSize(1);
  canvas.print(" inHg");

  // Temperature
  canvas.setCursor(10, 100);
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextSize(1);
  canvas.print("Temp: ");
  canvas.setTextColor(TFT_ORANGE);
  float temp = pressureSensor.getTemperature();
  canvas.print(temp, 1);
  canvas.print("C ");
  canvas.setTextColor(TFT_YELLOW);
  float tempF = temp * 9.0 / 5.0 + 32.0;
  canvas.print(tempF, 1);
  canvas.print("F");

  // Calibration bounds
  canvas.setCursor(10, 114);
  canvas.setTextColor(TFT_GRAY);
  canvas.print("Min:");
  canvas.print(breathData.getMinDelta(), 0);
  canvas.print(" Max:");
  canvas.print(breathData.getMaxDelta(), 0);
}
