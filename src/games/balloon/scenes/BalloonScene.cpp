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

// Background tile (width for scrolling, height matches screen)
static const int TILE_WIDTH = 64;
static const int TILE_HEIGHT = SCREEN_HEIGHT;

static const uint32_t SUNSET_BANDS[] = {
  0x962730,
  0xcc2e2b,
  0xf45327,
  0xfa7e38,
  0xfa9c78,
  0xfa6859,
  0xfb9e75,
  0xfad28d,
  0xf8a755
};
static const int NUM_BANDS = sizeof(SUNSET_BANDS) / sizeof(SUNSET_BANDS[0]);

static const uint32_t CLOUD_COLOR = 0xfa946e;

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
static const int BALLOON_KNOT_WIDTH = 4;
static const int BALLOON_KNOT_HEIGHT = 4;
static const int BALLOON_KNOT_OFFSET = -2;

static const uint32_t BALLOON_COLOR = 0xFF5050;
static const uint32_t BALLOON_OUTLINE_COLOR = 0x321414;
static const uint32_t BALLOON_HIGHLIGHT_COLOR = 0xFFC8C8;
static const uint32_t STRING_COLOR = 0x64503C;

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
  _bgTile->createSprite(TILE_WIDTH, TILE_HEIGHT);
  generateBackgroundTile();
}

void BalloonScene::generateBackgroundTile() {
  const int bandHeight = TILE_HEIGHT / NUM_BANDS;

  for (int y = 0; y < TILE_HEIGHT; y++) {
    int band = y / bandHeight;
    if (band >= NUM_BANDS) band = NUM_BANDS - 1;

    uint32_t hex = SUNSET_BANDS[band];
    uint8_t r = (hex >> 16) & 0xFF;
    uint8_t g = (hex >> 8) & 0xFF;
    uint8_t b = hex & 0xFF;
    uint16_t rgb565 = Display::rgb565(r, g, b);
    _bgTile->drawFastHLine(0, y, TILE_WIDTH, rgb565);
  }

  uint8_t cloudR = (CLOUD_COLOR >> 16) & 0xFF;
  uint8_t cloudG = (CLOUD_COLOR >> 8) & 0xFF;
  uint8_t cloudB = CLOUD_COLOR & 0xFF;
  uint16_t cloudColor = Display::rgb565(cloudR, cloudG, cloudB);

  // Cloud 1 (upper area - around 1/6 down screen)
  _bgTile->fillEllipse(16, 30, 12, 6, cloudColor);
  _bgTile->fillEllipse(12, 32, 8, 5, cloudColor);
  _bgTile->fillEllipse(22, 32, 9, 5, cloudColor);

  // Cloud 2 (middle area - around 1/2 down screen)
  _bgTile->fillEllipse(50, 70, 10, 5, cloudColor);
  _bgTile->fillEllipse(45, 72, 7, 4, cloudColor);
  _bgTile->fillEllipse(56, 71, 6, 4, cloudColor);
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

  uint16_t outlineColor = Display::rgb565(
    (BALLOON_OUTLINE_COLOR >> 16) & 0xFF,
    (BALLOON_OUTLINE_COLOR >> 8) & 0xFF,
    BALLOON_OUTLINE_COLOR & 0xFF
  );

  int knotY = adjustedY + height + BALLOON_KNOT_OFFSET;

  // Draw all outlines first
  canvas.fillEllipse(x, adjustedY, width + 1, height + 1, outlineColor);
  canvas.fillTriangle(x - BALLOON_KNOT_WIDTH - 1, knotY,
                      x + BALLOON_KNOT_WIDTH + 1, knotY,
                      x, knotY + BALLOON_KNOT_HEIGHT + 3,
                      outlineColor);

  // Draw all fills on top
  canvas.fillEllipse(x, adjustedY, width, height, color);

  uint16_t highlight = Display::rgb565(
    (BALLOON_HIGHLIGHT_COLOR >> 16) & 0xFF,
    (BALLOON_HIGHLIGHT_COLOR >> 8) & 0xFF,
    BALLOON_HIGHLIGHT_COLOR & 0xFF
  );
  int highlightH = (int)(BALLOON_HIGHLIGHT_H * squashFactor);
  int highlightW = (int)(BALLOON_HIGHLIGHT_W * stretchFactor);
  int highlightY = adjustedY + (int)(BALLOON_HIGHLIGHT_Y_OFFSET * squashFactor);
  canvas.fillEllipse(x + BALLOON_HIGHLIGHT_X_OFFSET, highlightY, highlightW, highlightH, highlight);

  canvas.fillTriangle(x - BALLOON_KNOT_WIDTH, knotY,
                      x + BALLOON_KNOT_WIDTH, knotY,
                      x, knotY + BALLOON_KNOT_HEIGHT + 2,
                      color);

  // Draw string starting from bottom of knot
  int stringStartY = knotY + BALLOON_KNOT_HEIGHT + 4;
  uint16_t stringColor = Display::rgb565(
    (STRING_COLOR >> 16) & 0xFF,
    (STRING_COLOR >> 8) & 0xFF,
    STRING_COLOR & 0xFF
  );
  drawBalloonString(canvas, x, stringStartY, stringVelocity, stringColor);
}

void BalloonScene::update(float dt) {
  // Update scroll position (scroll left)
  _scrollX += SCROLL_SPEED * dt;
  if (_scrollX >= TILE_WIDTH) {
    _scrollX -= TILE_WIDTH;
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
  // Draw tiled background with horizontal scroll offset (no vertical tiling)
  int offsetX = -(int)_scrollX;
  for (int tx = offsetX; tx < SCREEN_WIDTH; tx += TILE_WIDTH) {
    _bgTile->pushSprite(&canvas, tx, 0);
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
  uint16_t balloonColor = Display::rgb565(
    (BALLOON_COLOR >> 16) & 0xFF,
    (BALLOON_COLOR >> 8) & 0xFF,
    BALLOON_COLOR & 0xFF
  );
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
