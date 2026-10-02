#include "SceneManager.h"

#include "BaseScene.h"
#include "GamePlayScene.h"
#include "TitleScene.h"

#include <cassert>
#include <utility>

SceneManager::SceneManager(Input *input, DirectXCommon *dxCommon,
                           SrvManager *srvManager, ModelCommon *modelCommon)
    : input(input), dxCommon(dxCommon), srvManager(srvManager),
      modelCommon(modelCommon) {
  assert(input);
  assert(dxCommon);
  assert(srvManager);
  assert(modelCommon);
}

SceneManager::~SceneManager() = default;

void SceneManager::Initialize(SceneType firstScene) {
  ChangeScene(firstScene);
  ApplySceneChange();
}

void SceneManager::Update() {
  ApplySceneChange();

  if (currentScene) {
    currentScene->Update();
  }
}

void SceneManager::Draw() {
  if (currentScene) {
    currentScene->Draw();
  }
}

void SceneManager::Finalize() {
  // 初期化前の遷移待ちシーンは、そのまま破棄する。
  nextScene.reset();

  if (currentScene) {
    currentScene->Finalize();
    currentScene.reset();
  }
}

void SceneManager::ChangeScene(SceneType sceneType) {
  // Update中のシーンをその場で破棄せず、次フレーム用に予約する。
  nextScene = CreateScene(sceneType);
  assert(nextScene);
}

std::unique_ptr<BaseScene> SceneManager::CreateScene(SceneType sceneType) {
  switch (sceneType) {
  case SceneType::Title:
    return std::make_unique<TitleScene>(this, input);

  case SceneType::GamePlay:
    return std::make_unique<GamePlayScene>(this, input, dxCommon, srvManager,
                                           modelCommon);
  }

  assert(false && "Unknown scene type");
  return nullptr;
}

void SceneManager::ApplySceneChange() {
  if (!nextScene) {
    return;
  }

  if (currentScene) {
    currentScene->Finalize();
  }

  currentScene = std::move(nextScene);
  currentScene->Initialize();
}
