#include "TitleScene.h"

#include "Input.h"
#include "SceneManager.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "Window.h"

#include <cassert>

TitleScene::TitleScene(SceneManager *sceneManager, Input *input)
    : sceneManager(sceneManager), input(input) {
  assert(sceneManager);
  assert(input);
}

TitleScene::~TitleScene() = default;

void TitleScene::Initialize() {
  titleSprite = std::make_unique<Sprite>();
  titleSprite->Initialize(SpriteCommon::GetInstance(),
                          "resources/monsterBall.png");
  titleSprite->SetPosition({0.0f, 0.0f});
  titleSprite->SetSize({static_cast<float>(Window::kClientWidth),
                        static_cast<float>(Window::kClientHeight)});
}

void TitleScene::Update() {
  titleSprite->Update();

  if (input->TriggerKey(DIK_SPACE)) {
    sceneManager->ChangeScene(SceneType::GamePlay);
  }
}

void TitleScene::Draw() {
  SpriteCommon::GetInstance()->CreatePrimitiveTopology();
  titleSprite->Draw();
}

void TitleScene::Finalize() { titleSprite.reset(); }
