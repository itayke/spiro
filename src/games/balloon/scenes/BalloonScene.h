#ifndef BALLOON_SCENE_H
#define BALLOON_SCENE_H

#include "core/scenes/SceneBase.h"
#include "config.h"

class BalloonScene : public SceneBase {
public:
  BalloonScene();
  ~BalloonScene();

  void init() override;
  void update(float dt) override;
  void draw(Canvas& canvas) override;
  int getFps() const override { return 50; }

private:
  void generateBackgroundTile();
  void drawBalloon(Canvas& canvas, int x, int y, uint16_t color, float squash, int8_t squashDir, float stringVelocity);
  void drawBalloonString(Canvas& canvas, int x, int y, float stringVelocity, uint16_t stringColor);

  // Background
  LGFX_Sprite* _bgTile;
  float _scrollX;

  // Balloon state
  float _smoothedNormalized;
  float _deltaNormalizedY;

  // String physics simulation
  float _stringEndY;        // Simulated string end Y position
  float _stringEndVelocity; // String end Y velocity

  // Score
  unsigned long _score;
};

#endif // BALLOON_SCENE_H
