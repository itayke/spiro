#ifndef DIAGNOSTIC_SCENE_H
#define DIAGNOSTIC_SCENE_H

#include "core/scenes/SceneBase.h"

class DiagnosticScene : public SceneBase {
public:
  DiagnosticScene();

  void update(float dt) override;
  void draw(Canvas& canvas) override;
  int getFps() const override { return 10; }

private:
  float _pressureDelta;
};

#endif // DIAGNOSTIC_SCENE_H
