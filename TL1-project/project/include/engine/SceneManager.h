#pragma once

#include <memory>

class BaseScene;
class DirectXCommon;
class Input;
class ModelCommon;
class SrvManager;

enum class SceneType {
  Title,
  GamePlay,
};

class SceneManager {
public:
  SceneManager(Input *input, DirectXCommon *dxCommon, SrvManager *srvManager,
               ModelCommon *modelCommon);
  ~SceneManager();

  SceneManager(const SceneManager &) = delete;
  SceneManager &operator=(const SceneManager &) = delete;

  void Initialize(SceneType firstScene);
  void Update();
  void Draw();
  void Finalize();

  void ChangeScene(SceneType sceneType);

private:
  std::unique_ptr<BaseScene> CreateScene(SceneType sceneType);
  void ApplySceneChange();

  Input *input = nullptr;
  DirectXCommon *dxCommon = nullptr;
  SrvManager *srvManager = nullptr;
  ModelCommon *modelCommon = nullptr;

  std::unique_ptr<BaseScene> currentScene;
  std::unique_ptr<BaseScene> nextScene;
};
