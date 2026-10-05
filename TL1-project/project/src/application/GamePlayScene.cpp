#include "GamePlayScene.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>

#include "Camera.h"
#include "Enemy.h"
#include "Input.h"
#include "MapChipField.h"
#include "Model.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Player.h"
#include "SceneManager.h"
#include "SkyBox.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "Window.h"

GamePlayScene::GamePlayScene(SceneManager *sceneManager, Input *input,
                             DirectXCommon *dxCommon, SrvManager *srvManager,
                             ModelCommon *modelCommon)
    : sceneManager(sceneManager), input(input), dxCommon(dxCommon),
      srvManager(srvManager), modelCommon(modelCommon) {
  assert(sceneManager);
  assert(input);
  assert(dxCommon);
  assert(srvManager);
  assert(modelCommon);
}

GamePlayScene::~GamePlayScene() = default;

void GamePlayScene::Initialize() {
  fadeAlpha = 1.0f;

  mapChipField = std::make_unique<MapChipField>();
  mapChipField->LoadMapChipCsv("resources/maptip.csv");

  blockModel = std::make_unique<Model>();
  blockModel->initialize(modelCommon, "resources/block", "block.obj");

  goalModel = std::make_unique<Model>();
  goalModel->initialize(modelCommon, "resources/goal", "goal.obj");

  playerModel = std::make_unique<Model>();
  playerModel->initialize(modelCommon, "resources/player", "player.obj");

  enemyModel = std::make_unique<Model>();
  enemyModel->initialize(modelCommon, "resources/enemy3", "enemy3.obj");

  camera = std::make_unique<Camera>();
  camera->SetRotate({0.0f, 0.0f, 0.0f});
  camera->SetTranslate(cameraPosition);
  camera->SetFarClip(300.0f);
  camera->Update();

  skyBox = std::make_unique<SkyBox>();
  skyBox->Initialize(dxCommon, "resources/sky_sphere.png");
  skyBox->Update(camera.get());

  fadeSprite = std::make_unique<Sprite>();
  fadeSprite->Initialize(SpriteCommon::GetInstance(), "resources/white.png");
  fadeSprite->SetPosition({0.0f, 0.0f});
  fadeSprite->SetSize({static_cast<float>(Window::kClientWidth),
                       static_cast<float>(Window::kClientHeight)});
  fadeSprite->SetColor({0.0f, 0.0f, 0.0f, fadeAlpha});
  fadeSprite->Update();

  GenerateMapObjects();

  player = std::make_unique<Player>();
  const Vector::Vector3 playerPosition =
      mapChipField->GetMapChipPositionByIndex(2, 11);
  player->Initialize(playerModel.get(), camera.get(), input,
                     mapChipField.get(), playerPosition);

  GenerateEnemies();
}

std::unique_ptr<Object3d>
GamePlayScene::CreateMapObject(Model *model,
                               const Vector::Vector3 &position) {
  auto object = std::make_unique<Object3d>();
  object->Initialize(Object3dCommon::GetInstance());
  object->SetModel(model);
  object->SetCamera(camera.get());
  object->SetTranslate(position);
  object->Update();
  return object;
}

void GamePlayScene::GenerateMapObjects() {
  for (uint32_t y = 0; y < mapChipField->GetNumBlockVertical(); ++y) {
    for (uint32_t x = 0; x < mapChipField->GetNumBlockHorizontal(); ++x) {
      const MapChipType type = mapChipField->GetMapChipTypeByIndex(x, y);
      const Vector::Vector3 position =
          mapChipField->GetMapChipPositionByIndex(x, y);

      switch (type) {
      case MapChipType::Block:
        blockObjects.emplace_back(CreateMapObject(blockModel.get(), position));
        break;

      case MapChipType::Goal:
        goalObjects.emplace_back(CreateMapObject(goalModel.get(), position));
        break;

      case MapChipType::MovingPlatform: {
        MovingPlatform platform;
        platform.startPosition = position;
        platform.object = CreateMapObject(blockModel.get(), position);
        movingPlatforms.emplace_back(std::move(platform));
        break;
      }

      case MapChipType::Blank:
        break;
      }
    }
  }
}

void GamePlayScene::GenerateEnemies() {
  const auto spawnEnemy = [this](uint32_t x, uint32_t y) {
    auto enemy = std::make_unique<Enemy>();
    enemy->Initialize(enemyModel.get(), camera.get(), mapChipField.get(),
                      mapChipField->GetMapChipPositionByIndex(x, y));
    enemies.emplace_back(std::move(enemy));
  };

  spawnEnemy(16, 18);
  spawnEnemy(24, 18);
  spawnEnemy(32, 18);
  spawnEnemy(27, 11);
  spawnEnemy(19, 4);
  spawnEnemy(32, 4);
}

void GamePlayScene::CheckPlayerEnemyCollisions() {
  const PlayerAABB playerAABB = player->GetAABB();
  for (auto enemyIterator = enemies.begin();
       enemyIterator != enemies.end();) {
    const EnemyAABB enemyAABB = (*enemyIterator)->GetAABB();
    const bool isColliding =
        playerAABB.min.x < enemyAABB.max.x &&
        playerAABB.max.x > enemyAABB.min.x &&
        playerAABB.min.y < enemyAABB.max.y &&
        playerAABB.max.y > enemyAABB.min.y &&
        playerAABB.min.z < enemyAABB.max.z &&
        playerAABB.max.z > enemyAABB.min.z;

    if (isColliding) {
      if (player->IsAttacking()) {
        enemyIterator = enemies.erase(enemyIterator);
        continue;
      }

      if (player->CanBeHit()) {
        RespawnPlayer();
        return;
      }
    }

    ++enemyIterator;
  }
}

void GamePlayScene::RespawnPlayer() {
  player->Respawn();

  const Vector::Vector3 &position = player->GetPosition();
  cameraPosition.x = std::clamp(
      position.x, 20.0f,
      MapChipField::kBlockWidth *
              static_cast<float>(MapChipField::kNumBlockHorizontal - 1) -
          20.0f);
  cameraPosition.y = std::clamp(
      position.y, 10.0f,
      MapChipField::kBlockHeight *
              static_cast<float>(MapChipField::kNumBlockVertical - 1) -
          10.0f);
  camera->SetTranslate(cameraPosition);
  camera->Update();
}

void GamePlayScene::UpdateCamera() {
  assert(player);

  constexpr float kFollowRate = 0.1f;
  const Vector::Vector3 &playerPosition = player->GetPosition();
  const float targetX = playerPosition.x;
  const float targetY = playerPosition.y;

  cameraPosition.x += (targetX - cameraPosition.x) * kFollowRate;
  cameraPosition.y += (targetY - cameraPosition.y) * kFollowRate;

  cameraPosition.x = std::clamp(
      cameraPosition.x, 20.0f,
      MapChipField::kBlockWidth *
              static_cast<float>(MapChipField::kNumBlockHorizontal - 1) -
          20.0f);
  cameraPosition.y = std::clamp(
      cameraPosition.y, 10.0f,
      MapChipField::kBlockHeight *
              static_cast<float>(MapChipField::kNumBlockVertical - 1) -
          10.0f);

  camera->SetTranslate(cameraPosition);
  camera->Update();
}

void GamePlayScene::Update() {
  if (input->TriggerKey(DIK_ESCAPE)) {
    sceneManager->ChangeScene(SceneType::Title);
    return;
  }

  movingPlatformTime += 1.0f / 60.0f;
  const float movingOffset = std::sin(movingPlatformTime) * 3.0f;
  for (MovingPlatform &platform : movingPlatforms) {
    const Vector::Vector3 previousPosition = platform.object->GetTranslate();
    Vector::Vector3 position = platform.startPosition;
    position.x += movingOffset;
    platform.moveDelta = {
        position.x - previousPosition.x,
        position.y - previousPosition.y,
        position.z - previousPosition.z,
    };
    platform.object->SetTranslate(position);
  }

  player->Update();
  for (const std::unique_ptr<Enemy> &enemy : enemies) {
    enemy->Update();
  }
  CheckPlayerEnemyCollisions();
  for (const MovingPlatform &platform : movingPlatforms) {
    const Vector::Vector3 &position = platform.object->GetTranslate();
    const MapChipRect platformRect = {
        position.x - MapChipField::kBlockWidth / 2.0f,
        position.x + MapChipField::kBlockWidth / 2.0f,
        position.y - MapChipField::kBlockHeight / 2.0f,
        position.y + MapChipField::kBlockHeight / 2.0f,
    };
    player->ResolveMovingPlatformCollision(platformRect,
                                           platform.moveDelta);
  }
  UpdateCamera();
  skyBox->Update(camera.get());

  for (const std::unique_ptr<Object3d> &object : blockObjects) {
    object->Update();
  }
  for (const std::unique_ptr<Object3d> &object : goalObjects) {
    object->Update();
  }
  for (MovingPlatform &platform : movingPlatforms) {
    platform.object->Update();
  }

  constexpr float kFadeSpeed = 2.0f;
  fadeAlpha -= kFadeSpeed / 60.0f;
  if (fadeAlpha < 0.0f) {
    fadeAlpha = 0.0f;
  }
  fadeSprite->SetColor({0.0f, 0.0f, 0.0f, fadeAlpha});
  fadeSprite->Update();
}

void GamePlayScene::Draw() {
  skyBox->CreatePrimitiveTopology();
  skyBox->Draw();

  Object3dCommon::GetInstance()->CreatePrimitiveTopology();

  for (const std::unique_ptr<Object3d> &object : blockObjects) {
    object->Draw();
  }
  for (const std::unique_ptr<Object3d> &object : goalObjects) {
    object->Draw();
  }
  for (const MovingPlatform &platform : movingPlatforms) {
    platform.object->Draw();
  }
  player->Draw();
  for (const std::unique_ptr<Enemy> &enemy : enemies) {
    enemy->Draw();
  }

  SpriteCommon::GetInstance()->CreatePrimitiveTopology();
  fadeSprite->Draw();
}

void GamePlayScene::Finalize() {
  fadeSprite.reset();
  enemies.clear();
  player.reset();
  movingPlatforms.clear();
  goalObjects.clear();
  blockObjects.clear();
  skyBox.reset();
  camera.reset();
  playerModel.reset();
  enemyModel.reset();
  goalModel.reset();
  blockModel.reset();
  mapChipField.reset();
}
