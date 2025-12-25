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
  struct Collectible {
    float x;
    float y;
    bool active;
    bool collecting;
    float fadeTimer;
  };

  void generateBackgroundTile();
  void drawBalloon(Canvas& canvas, int x, int y, uint16_t color, float squash, int8_t squashDir, float stringVelocity);
  void drawBalloonString(Canvas& canvas, int x, int y, float stringVelocity, uint16_t stringColor);
  void drawCollectible(Canvas& canvas, float x, float y, float alpha);
  void spawnCollectible(int index);
  void checkCollectibleCollision(int balloonX, int balloonY);

  // Background
  LGFX_Sprite* _bgTile;
  float _scrollX;

  // Collectible sprites (keyframes for fade animation)
  static const int COLLECTIBLE_KEYFRAMES = 5;
  LGFX_Sprite* _collectibleSprites[COLLECTIBLE_KEYFRAMES];
  void createCollectibleSprites();

  // Balloon state
  float _smoothedNormalized;
  float _deltaNormalizedY;

  // String physics simulation
  float _stringEndY;        // Simulated string end Y position
  float _stringEndVelocity; // String end Y velocity

  // Collectibles
  static const int MAX_COLLECTIBLES = 5;
  Collectible _collectibles[MAX_COLLECTIBLES];

  // Score
  unsigned long _score;
};

#endif // BALLOON_SCENE_H
