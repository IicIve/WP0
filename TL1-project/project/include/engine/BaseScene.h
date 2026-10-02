#pragma once

class BaseScene {
public:
  BaseScene() = default;
  virtual ~BaseScene();

  BaseScene(const BaseScene &) = delete;
  BaseScene &operator=(const BaseScene &) = delete;

  virtual void Initialize() = 0;
  virtual void Update() = 0;
  virtual void Draw() = 0;
  virtual void Finalize() = 0;
};
