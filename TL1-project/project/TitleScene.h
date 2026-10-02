#pragma once

#include <memory>

#include "BaseScene.h"

class Sprite;
class Input;
class SceneManager;

class TitleScene final : public BaseScene {
public:
  TitleScene(SceneManager *sceneManager, Input *input);
  ~TitleScene() override;

  TitleScene(const TitleScene &) = delete;
  TitleScene &operator=(const TitleScene &) = delete;

  void Initialize() override;
  void Update() override;
  void Draw() override;
  void Finalize() override;

private:
  SceneManager *sceneManager = nullptr;
  Input *input = nullptr;
  std::unique_ptr<Sprite> titleSprite;
};
