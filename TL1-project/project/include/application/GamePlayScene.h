#pragma once

#include <memory>
#include <vector>

#include "BaseScene.h"
#include "Vector.h"

class Camera;
class DirectXCommon;
class Enemy;
class Input;
class MapChipField;
class Model;
class ModelCommon;
class Object3d;
class Player;
class SceneManager;
class SkyBox;
class Sprite;
class SrvManager;

class GamePlayScene final : public BaseScene {
public:
  GamePlayScene(SceneManager *sceneManager, Input *input,
                DirectXCommon *dxCommon, SrvManager *srvManager,
                ModelCommon *modelCommon);
  ~GamePlayScene() override;

  GamePlayScene(const GamePlayScene &) = delete;
  GamePlayScene &operator=(const GamePlayScene &) = delete;

  void Initialize() override;
  void Update() override;
  void Draw() override;
  void Finalize() override;

private:
  struct MovingPlatform {
    std::unique_ptr<Object3d> object;
    Vector::Vector3 startPosition{};
    Vector::Vector3 moveDelta{};
  };

  std::unique_ptr<Object3d> CreateMapObject(Model *model,
                                             const Vector::Vector3 &position);
  void GenerateMapObjects();
  void GenerateEnemies();
  void CheckPlayerEnemyCollisions();
  void RespawnPlayer();
  void UpdateCamera();

  SceneManager *sceneManager = nullptr;
  Input *input = nullptr;
  DirectXCommon *dxCommon = nullptr;
  SrvManager *srvManager = nullptr;
  ModelCommon *modelCommon = nullptr;

  std::unique_ptr<MapChipField> mapChipField;
  std::unique_ptr<Model> blockModel;
  std::unique_ptr<Model> goalModel;
  std::unique_ptr<Model> playerModel;
  std::unique_ptr<Model> enemyModel;
  std::unique_ptr<Camera> camera;
  std::unique_ptr<SkyBox> skyBox;
  std::unique_ptr<Sprite> fadeSprite;
  std::unique_ptr<Player> player;
  std::vector<std::unique_ptr<Enemy>> enemies;

  std::vector<std::unique_ptr<Object3d>> blockObjects;
  std::vector<std::unique_ptr<Object3d>> goalObjects;
  std::vector<MovingPlatform> movingPlatforms;

  Vector::Vector3 cameraPosition{20.0f, 16.0f, -45.0f};
  float movingPlatformTime = 0.0f;
  float fadeAlpha = 1.0f;
};
