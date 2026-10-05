#pragma once

#include <memory>

#include "BaseScene.h"

class Camera;
class DirectXCommon;
class Input;
class Model;
class ModelCommon;
class Object3d;
class SceneManager;
class SkyBox;
class Sprite;

class TitleScene final : public BaseScene {
public:
  TitleScene(SceneManager *sceneManager, Input *input, DirectXCommon *dxCommon,
             ModelCommon *modelCommon);
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
  DirectXCommon *dxCommon = nullptr;
  ModelCommon *modelCommon = nullptr;

  std::unique_ptr<Camera> camera;
  std::unique_ptr<SkyBox> skyBox;
  std::unique_ptr<Model> titleModel;
  std::unique_ptr<Object3d> titleObject;
  std::unique_ptr<Sprite> promptPanelSprite;
  std::unique_ptr<Sprite> promptSprite;
  std::unique_ptr<Sprite> fadeSprite;

  float animationTime = 0.0f;
  float fadeAlpha = 0.0f;
  bool isFadingOut = false;
};
