#ifndef LIVE_SCENE_H
#define LIVE_SCENE_H

#include "core/scenes/SceneBase.h"

class LiveScene : public SceneBase {
public:
  LiveScene();

  void update(float dt) override;
  void draw(Canvas& canvas) override;
  int getFps() const override { return 30; }

private:
  float _wavePhase;
  float _targetWaveHeight;
  float _currentWaveHeight;
};

#endif // LIVE_SCENE_H
