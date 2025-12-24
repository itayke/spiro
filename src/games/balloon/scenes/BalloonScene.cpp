#include "BalloonScene.h"
#include "core/hardware/BreathData.h"
#include "core/hardware/Display.h"

#ifndef SIMULATOR
  #include <Arduino.h>
#else
  #include "Platform.h"
#endif

// ========================================
// Configuration Constants
// ========================================

// Timing
static const float SCROLL_SPEED = 30.0f;  // pixels per second

// Background tile
static const int TILE_SIZE = 64;

// Sky colors (top to bottom gradient)
static const uint8_t SKY_TOP_R = 100;
static const uint8_t SKY_TOP_G = 150;
static const uint8_t SKY_TOP_B = 255;
static const uint8_t SKY_GRADIENT_R = 30;
static const uint8_t SKY_GRADIENT_G = 20;
static const uint8_t SKY_GRADIENT_B = 30;

// Cloud colors
static const uint8_t CLOUD_R = 240;
static const uint8_t CLOUD_G = 245;
static const uint8_t CLOUD_B = 255;

// Balloon position
static const float BALLOON_X_RATIO = 0.25f;
static const int BALLOON_Y_MARGIN = 12;
static const float SMOOTHING_FACTOR = 0.5f;

// String physics
static const int STRING_SEG1_LEN = 8;
static const int STRING_SEG2_LEN = 18;
static const float STRING_VELOCITY_LAG_X = 1.5f;       // How much velocity affects string curve (horizontal)
static const float STRING_VELOCITY_LAG_Y = 2.0f;       // How much velocity affects string curve (vertical)
static const float STRING_VELOCITY_POS_FACTOR = 0.5f;  // Reduce effect when velocity is negative
static const float STRING_WIND_AMPLITUDE = 3.0f;       // Wind sway amount
static const float STRING_WIND_SPEED = 9.0f;           // Wind oscillation speed
static const float STRING_WIND_SPEED_END = 8.0f;       // Different speed for end (more chaotic)
static const float STRING_CTRL_VELOCITY_MULT = 0.0f;   // Control point velocity multiplier
static const float STRING_END_VELOCITY_MULT = 1.0f;    // End point velocity multiplier
static const float STRING_BASE_OFFSET_X = -4.0f;       // Base X offset (negative = left/behind balloon)
static const float STRING_SPRING_FORCE = 75.0f;        // How strongly string follows balloon Y
static const float STRING_DRAG = 0.85f;                // Velocity damping (0-1, lower = more drag)
static const float BALLOON_SPEED_SQUASH_FACTOR = 0.5f;

// Balloon dimensions
static const int BALLOON_WIDTH = 9;
static const int BALLOON_HEIGHT = 12;
static const int BALLOON_HIGHLIGHT_X_OFFSET = -3;
static const int BALLOON_HIGHLIGHT_Y_OFFSET = -4;
static const int BALLOON_HIGHLIGHT_W = 2;
static const int BALLOON_HIGHLIGHT_H = 3;
static const int BALLOON_KNOT_WIDTH = 2;
static const int BALLOON_KNOT_HEIGHT = 3;

// Balloon colors
static const uint8_t BALLOON_R = 255;
static const uint8_t BALLOON_G = 80;
static const uint8_t BALLOON_B = 80;
static const uint8_t BALLOON_HIGHLIGHT_R = 255;
static const uint8_t BALLOON_HIGHLIGHT_G = 200;
static const uint8_t BALLOON_HIGHLIGHT_B = 200;
static const uint8_t STRING_R = 100;
static const uint8_t STRING_G = 80;
static const uint8_t STRING_B = 60;

// ========================================

BalloonScene::BalloonScene()
  : _bgTile(nullptr)
  , _scrollX(0)
  , _smoothedNormalized(0)
  , _deltaNormalizedY(0)
  , _stringEndY(0)
  , _stringEndVelocity(0)
  , _score(0) {
}

BalloonScene::~BalloonScene() {
  if (_bgTile) {
    _bgTile->deleteSprite();
    delete _bgTile;
    _bgTile = nullptr;
  }
}

void BalloonScene::init() {
  _bgTile = new LGFX_Sprite(&display.getLcd());
  _bgTile->setColorDepth(16);
  _bgTile->createSprite(TILE_SIZE, TILE_SIZE);
  generateBackgroundTile();
}

void BalloonScene::generateBackgroundTile() {
  // Sky gradient base
  for (int y = 0; y < TILE_SIZE; y++) {
    uint8_t r = SKY_TOP_R - (y * SKY_GRADIENT_R / TILE_SIZE);
    uint8_t g = SKY_TOP_G - (y * SKY_GRADIENT_G / TILE_SIZE);
    uint8_t b = SKY_TOP_B - (y * SKY_GRADIENT_B / TILE_SIZE);
    uint16_t color = Display::rgb565(r, g, b);
    _bgTile->drawFastHLine(0, y, TILE_SIZE, color);
  }

  // Add some simple cloud shapes
  uint16_t cloudColor = Display::rgb565(CLOUD_R, CLOUD_G, CLOUD_B);

  // Cloud 1
  _bgTile->fillEllipse(16, 20, 12, 6, cloudColor);
  _bgTile->fillEllipse(12, 22, 8, 5, cloudColor);
  _bgTile->fillEllipse(22, 22, 9, 5, cloudColor);

  // Cloud 2
  _bgTile->fillEllipse(50, 45, 10, 5, cloudColor);
  _bgTile->fillEllipse(45, 47, 7, 4, cloudColor);
  _bgTile->fillEllipse(56, 46, 6, 4, cloudColor);
}

void BalloonScene::drawBalloonString(Canvas& canvas, int x, int stringStartY, float stringVelocity, uint16_t stringColor) {
  // Time-based wind effect (use millis for continuous animation)
  float time = millis() / 1000.0f;

  // Wind sway using sine waves with different frequencies
  float windControlX = sin(time * STRING_WIND_SPEED) * STRING_WIND_AMPLITUDE;
  float windEndX = sin(time * STRING_WIND_SPEED_END + 1.5f) * STRING_WIND_AMPLITUDE;

  // Velocity-based lag (when string moves down, it lags up and back diagonally)
  // Use simulated string velocity for more realistic physics
  float velocityFactored = stringVelocity > 0 ? stringVelocity * STRING_VELOCITY_POS_FACTOR : stringVelocity;
  float velocityLagX = velocityFactored * STRING_VELOCITY_LAG_X;  // Horizontal drag
  float velocityLagY = velocityFactored * STRING_VELOCITY_LAG_Y; // Vertical drag (negative = opposite direction)

  // Calculate bezier curve points
  int startX = x;  // Attach at balloon center
  int startY = stringStartY;

  // Control point offset to left + reduced displacement (less responsive)
  int controlX = x + (int)((STRING_BASE_OFFSET_X + velocityLagX) * STRING_CTRL_VELOCITY_MULT + windControlX);
  int controlY = stringStartY + STRING_SEG1_LEN + (int)(velocityLagY * STRING_CTRL_VELOCITY_MULT);

  // End point offset to left + full displacement (most responsive, whip effect)
  int endX = x + (int)((STRING_BASE_OFFSET_X + velocityLagX) * STRING_END_VELOCITY_MULT + windEndX);
  int endY = stringStartY + STRING_SEG2_LEN + (int)(velocityLagY * STRING_END_VELOCITY_MULT);

  // Draw string as bezier curve
  canvas.drawBezier(startX, startY, controlX, controlY, endX, endY, stringColor);
}

void BalloonScene::drawBalloon(Canvas& canvas, int x, int y, uint16_t color, float squash, int8_t squashDir, float stringVelocity) {
  // Calculate squashed dimensions
  float squashFactor = 1.0f - (squash * 2.0f);
  float stretchFactor = 1.0f + (squash * 1.6f);

  int height = (int)(BALLOON_HEIGHT * squashFactor);
  int width = (int)(BALLOON_WIDTH * stretchFactor);

  // Adjust Y position based on squash direction
  int adjustedY = y;
  if (squash > 0) {
    if (squashDir > 0) {
      adjustedY = y + (BALLOON_HEIGHT - height) / 2;
    } else if (squashDir < 0) {
      adjustedY = y - (BALLOON_HEIGHT - height) / 2;
    } else {
      adjustedY = y;
    }
  }

  // Balloon body (ellipse)
  canvas.fillEllipse(x, adjustedY, width, height, color);

  // Highlight (scale with squash)
  uint16_t highlight = Display::rgb565(BALLOON_HIGHLIGHT_R, BALLOON_HIGHLIGHT_G, BALLOON_HIGHLIGHT_B);
  int highlightH = (int)(BALLOON_HIGHLIGHT_H * squashFactor);
  int highlightW = (int)(BALLOON_HIGHLIGHT_W * stretchFactor);
  int highlightY = adjustedY + (int)(BALLOON_HIGHLIGHT_Y_OFFSET * squashFactor);
  canvas.fillEllipse(x + BALLOON_HIGHLIGHT_X_OFFSET, highlightY, highlightW, highlightH, highlight);

  // Knot at bottom
  int knotY = adjustedY + height - 2;
  canvas.fillTriangle(x - BALLOON_KNOT_WIDTH, knotY,
                      x + BALLOON_KNOT_WIDTH, knotY,
                      x, knotY + BALLOON_KNOT_HEIGHT + 2,
                      color);

  // Draw string starting from bottom of knot
  int stringStartY = knotY + BALLOON_KNOT_HEIGHT + 2;
  uint16_t stringColor = Display::rgb565(STRING_R, STRING_G, STRING_B);
  drawBalloonString(canvas, x, stringStartY, stringVelocity, stringColor);
}

void BalloonScene::update(float dt) {
  // Update scroll position (scroll left)
  _scrollX += SCROLL_SPEED * dt;
  if (_scrollX >= TILE_SIZE) {
    _scrollX -= TILE_SIZE;
  }

  // Smooth breath input
  float targetNormalized = breathData.getNormalizedBreathRaw();
  _deltaNormalizedY = targetNormalized - _smoothedNormalized;
  _smoothedNormalized += _deltaNormalizedY * SMOOTHING_FACTOR;

  // String physics: simulate string end following balloon Y position
  // String end Y tries to follow balloon's normalized Y with spring physics
  float balloonY = _smoothedNormalized;
  float stringDelta = balloonY - _stringEndY;

  // Apply spring force (delta pushes velocity toward balloon)
  _stringEndVelocity += stringDelta * STRING_SPRING_FORCE * dt;

  // Apply drag to velocity
  _stringEndVelocity *= STRING_DRAG;

  // Update string end position
  _stringEndY += _stringEndVelocity * dt;

  // Update score (1 point per frame for now)
  _score++;
}

void BalloonScene::draw(Canvas& canvas) {
  // Draw tiled background with scroll offset
  int offsetX = -(int)_scrollX;
  for (int tx = offsetX; tx < SCREEN_WIDTH; tx += TILE_SIZE) {
    for (int ty = 0; ty < SCREEN_HEIGHT; ty += TILE_SIZE) {
      _bgTile->pushSprite(&canvas, tx, ty);
    }
  }

  // Calculate balloon position
  int balloonX = (int)(SCREEN_WIDTH * BALLOON_X_RATIO);
  // Apply squash based on vertical speed, with minimal buffer
  float squash = _deltaNormalizedY > 0 ? 
    (std::max(_deltaNormalizedY - 0.1f, 0.0f)) * BALLOON_SPEED_SQUASH_FACTOR : 
    (std::min(_deltaNormalizedY + 0.1f, 0.0f)) * BALLOON_SPEED_SQUASH_FACTOR;
  float normalized = _smoothedNormalized + squash;
  int centerY = SCREEN_HEIGHT / 2;
  int maxDisplacement = (SCREEN_HEIGHT / 2) - BALLOON_Y_MARGIN;

  // Calculate squash effect when exceeding bounds
  int8_t squashDir = 0;
  if (normalized > 1.0f) {
    squash = std::min(normalized - 1.0f, 1.0f);
    squashDir = 1;
  } else if (normalized < -1.0f) {
    squash = std::min(-normalized - 1.0f, 1.0f);
    squashDir = -1;
  }

  int balloonY = centerY - (int)(normalized * maxDisplacement);

  // Draw balloon with squash effect
  uint16_t balloonColor = Display::rgb565(BALLOON_R, BALLOON_G, BALLOON_B);
  drawBalloon(canvas, balloonX, balloonY, balloonColor, squash, squashDir, _stringEndVelocity);

  // Draw HUD - score on top-right, right-justified
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextSize(1);

  char scoreText[16];
  snprintf(scoreText, sizeof(scoreText), "%lu", _score);
  int textWidth = strlen(scoreText) * 6;
  canvas.setCursor(SCREEN_WIDTH - textWidth - 4, 4);
  canvas.print(scoreText);
}
