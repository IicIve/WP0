#pragma once

#include <memory>
#include <vector>

#include "Model.h"
#include "Vector.h"

class Camera;
class Cylinder;
class DirectXCommon;
class Input;
class KeyframeAnimation;
class ModelCommon;
class Object3d;
class ParticleManager;
class Ring;
class SkyBox;
class Sprite;
class SrvManager;

class GamePlayScene final {
public:
  GamePlayScene(Input *input, DirectXCommon *dxCommon, SrvManager *srvManager,
                ModelCommon *modelCommon);
  ~GamePlayScene();

  GamePlayScene(const GamePlayScene &) = delete;
  GamePlayScene &operator=(const GamePlayScene &) = delete;

  void Initialize();
  void Update();
  void Draw();
  void Finalize();

private:
  // Frameworkが所有する共通基盤。GamePlaySceneは借りるだけで解放しない。
  Input *input = nullptr;
  DirectXCommon *dxCommon = nullptr;
  SrvManager *srvManager = nullptr;
  ModelCommon *modelCommon = nullptr;

  // ゲームプレイ中だけ所有するリソース。
  Sprite *sprite = nullptr;
  Model *model = nullptr;
  Model *terrainModel = nullptr;
  Object3d *object3d = nullptr;
  Object3d *object3d2 = nullptr;
  Camera *camera = nullptr;
  SkyBox *skyBox = nullptr;
  ParticleManager *particleManager = nullptr;
  ParticleManager *particleManager2 = nullptr;
  ParticleManager *smokeManager = nullptr;
  ParticleManager *flashManager = nullptr;
  Ring *ring = nullptr;
  Cylinder *cylinder = nullptr;
  KeyframeAnimation *keyframeAnimation = nullptr;

  Model::Skeleton skeleton;
  Model::SkinCluster skinCluster;
  std::vector<std::unique_ptr<Object3d>> levelObjects;

  float flashLightTime = 0.0f;
  Vector3 debugCameraRotate{};
  Vector3 debugCameraTranslate{};
  static constexpr float kMouseRotateSensitivity = 0.002f;
  static constexpr float kMouseWheelSensitivity = 0.01f;
};
