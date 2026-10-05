#include "TitleScene.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>

#include "Camera.h"
#include "Input.h"
#include "Model.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "SceneManager.h"
#include "SkyBox.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "Window.h"

TitleScene::TitleScene(SceneManager *sceneManager, Input *input,
                       DirectXCommon *dxCommon,
                       ModelCommon *modelCommon)
    : sceneManager(sceneManager), input(input), dxCommon(dxCommon),
      modelCommon(modelCommon) {
  assert(sceneManager);
  assert(input);
  assert(dxCommon);
  assert(modelCommon);
}

TitleScene::~TitleScene() = default;

void TitleScene::Initialize() {
  animationTime = 0.0f;
  fadeAlpha = 0.0f;
  isFadingOut = false;

  camera = std::make_unique<Camera>();
  camera->SetRotate({0.3f, 0.0f, 0.0f});
  camera->SetTranslate({0.0f, 15.0f, -45.0f});
  camera->Update();

  skyBox = std::make_unique<SkyBox>();
  skyBox->Initialize(dxCommon, "resources/sky_sphere.png");
  skyBox->Update(camera.get());

  titleModel = std::make_unique<Model>();
  titleModel->initialize(modelCommon, "resources/title", "title.obj");

  titleObject = std::make_unique<Object3d>();
  titleObject->Initialize(Object3dCommon::GetInstance());
  titleObject->SetModel(titleModel.get());
  titleObject->SetCamera(camera.get());
  titleObject->SetScale({1.0f, 1.0f, 1.0f});
  titleObject->SetRotate({0.0f, 5.2f, 0.0f});
  titleObject->SetTranslate({-10.0f, -4.0f, 0.0f});
  titleObject->Update();

  promptPanelSprite = std::make_unique<Sprite>();
  promptPanelSprite->Initialize(SpriteCommon::GetInstance(),
                                "resources/white.png");
  promptPanelSprite->SetPosition({470.0f, 550.0f});
  promptPanelSprite->SetSize({340.0f, 95.0f});
  promptPanelSprite->SetColor({1.0f, 1.0f, 1.0f, 0.78f});

  promptSprite = std::make_unique<Sprite>();
  promptSprite->Initialize(SpriteCommon::GetInstance(),
                           "resources/pressSpace.png");
  promptSprite->SetPosition({490.0f, 560.0f});
  promptSprite->SetSize({300.0f, 75.0f});
  promptSprite->SetTextureLeftTop({0.0f, 95.0f});
  promptSprite->SetTextureSize({200.0f, 50.0f});

  fadeSprite = std::make_unique<Sprite>();
  fadeSprite->Initialize(SpriteCommon::GetInstance(), "resources/white.png");
  fadeSprite->SetPosition({0.0f, 0.0f});
  fadeSprite->SetSize({static_cast<float>(Window::kClientWidth),
                       static_cast<float>(Window::kClientHeight)});
  fadeSprite->SetColor({0.0f, 0.0f, 0.0f, fadeAlpha});

  promptPanelSprite->Update();
  promptSprite->Update();
  fadeSprite->Update();
}

void TitleScene::Update() {
  animationTime += 1.0f / 60.0f;

  const float bob = std::sin(animationTime * 1.5f) * 0.25f;
  const float sway = std::sin(animationTime * 0.7f) * 0.03f;
  titleObject->SetTranslate({-10.0f, -4.0f + bob, 0.0f});
  titleObject->SetRotate({0.0f, 5.2f + sway, 0.0f});
  titleObject->Update();
  skyBox->Update(camera.get());

  const float promptAlpha =
      0.55f + (std::sin(animationTime * 3.0f) + 1.0f) * 0.225f;
  promptSprite->SetColor({1.0f, 1.0f, 1.0f, promptAlpha});

  promptPanelSprite->Update();
  promptSprite->Update();

  if (!isFadingOut && input->TriggerKey(DIK_SPACE)) {
    isFadingOut = true;
  }

  if (isFadingOut) {
    constexpr float kFadeSpeed = 2.0f;
    fadeAlpha += kFadeSpeed / 60.0f;
    if (fadeAlpha > 1.0f) {
      fadeAlpha = 1.0f;
    }
    if (fadeAlpha >= 1.0f) {
      sceneManager->ChangeScene(SceneType::GamePlay);
    }
  }

  fadeSprite->SetColor({0.0f, 0.0f, 0.0f, fadeAlpha});
  fadeSprite->Update();
}

void TitleScene::Draw() {
  skyBox->CreatePrimitiveTopology();
  skyBox->Draw();

  Object3dCommon::GetInstance()->CreatePrimitiveTopology();
  titleObject->Draw();

  SpriteCommon::GetInstance()->CreatePrimitiveTopology();
  promptPanelSprite->Draw();
  promptSprite->Draw();
  fadeSprite->Draw();
}

void TitleScene::Finalize() {
  fadeSprite.reset();
  promptSprite.reset();
  promptPanelSprite.reset();
  titleObject.reset();
  titleModel.reset();
  skyBox.reset();
  camera.reset();
}
