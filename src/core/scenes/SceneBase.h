#ifndef SCENE_BASE_H
#define SCENE_BASE_H

#include "core/hardware/Display.h"

class SceneBase {
public:
  virtual ~SceneBase() = default;

  // One-time initialization (called once when scene is created)
  virtual void init() {}

  // Update logic (called every frame)
  // dt = time since last frame in seconds
  virtual void update(float dt) = 0;

  // Render to canvas (called every frame after update)
  virtual void draw(Canvas& canvas) = 0;

  // Target frames per second for this scene
  virtual int getFps() const { return 30; }
};

#endif // SCENE_BASE_H
