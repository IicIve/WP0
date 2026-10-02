#include <Windows.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <memory>
#include <string>

#include <json.hpp>

#include "externals/imgui/imgui.h"

#include "GamePlayScene.h"

#include "Camera.h"
#include "Cylinder.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "KeyframeAnimation.h"
#include "ModelManager.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "ParticleManager.h"
#include "Ring.h"
#include "SceneManager.h"
#include "SkyBox.h"
#include "Sprite.h"
#include "SpriteCommon.h"

namespace {

// レベルデータを格納するための構造体
struct ObjectData {
  std::string type;
  std::string name;

  struct Transform {
    Vector3 translation;
    Vector3 rotation;
    Vector3 scaling;
  };

  Transform transform;
  std::string file_name;
};

struct LevelData {
  std::string name;
  std::vector<ObjectData> objects;
};

} // namespace

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
  // ゲーム固有リソースの初期化
  sprite = new Sprite();
  terrainModel = new Model();
  model = new Model();
  object3d = new Object3d();
  object3d2 = new Object3d();
  camera = new Camera();
  skyBox = new SkyBox();
  particleManager = new ParticleManager();
  particleManager2 = new ParticleManager();
  smokeManager = new ParticleManager();
  flashManager = new ParticleManager();
  ring = new Ring();
  cylinder = new Cylinder();
  keyframeAnimation = new KeyframeAnimation();

  sprite->Initialize(SpriteCommon::GetInstance(), "resources/uvChecker.png");
  model->initialize(modelCommon, "resources", "walk.gltf");
  skeleton = model->CreateSkeleton(model->GetRootNode());
  skinCluster = model->CreateSkinCluster(dxCommon->GetDevice(), srvManager,
                                         skeleton, model->GetModelData());
  terrainModel->initialize(modelCommon, "resources", "terrain.obj");
  camera->SetRotate({0.18f, -1.57079633f, 0.0f});
  camera->SetTranslate({18.0f, 4.0f, 0.0f});
  object3d->Initialize(Object3dCommon::GetInstance());
  object3d->SetModel(model);
  object3d->SetCamera(camera);
  object3d2->Initialize(Object3dCommon::GetInstance());
  object3d2->SetModel(terrainModel);
  object3d2->SetCamera(camera);
  object3d2->SetTranslate({0.0f, -2.0f, 0.0f});

  skyBox->Initialize(dxCommon);
  ring->Initialize(dxCommon);
  cylinder->Initialize(dxCommon);

  particleManager->Initialize(dxCommon, srvManager, camera,
                              "resources/circle.png");
  particleManager->SetSpeed(0.0f);
  particleManager2->Initialize(dxCommon, srvManager, camera,
                               "resources/gradationLine.png",
                               ParticleManager::PrimitiveType::Ring);
  particleManager2->SetEmitCount(1);
  particleManager2->SetScale(0.5f);
  particleManager2->SetLength(0.5f);
  particleManager2->SetSpeed(0.0f);
  particleManager2->SetScaleVelocity(32.0f);
  smokeManager->Initialize(dxCommon, srvManager, camera, "resources/circle.png",
                           ParticleManager::PrimitiveType::Plane,
                           ParticleManager::BlendMode::Alpha);
  smokeManager->SetPlaneSize(0.5f, 0.5f);
  smokeManager->SetEmitCount(10);
  smokeManager->SetColor({0.3f, 0.3f, 0.3f, 0.7f});
  smokeManager->SetLifeTimeRange(2.0f, 3.5f);
  smokeManager->SetSpeedRange(0.2f, 0.8f);
  smokeManager->SetUniformScaleRange(0.8f, 1.4f);
  smokeManager->SetScaleVelocityRange(1.0f, 16.0f);
  flashManager->Initialize(dxCommon, srvManager, camera,
                           "resources/circle.png");
  flashManager->SetPlaneSize(0.5f, 0.5f);
  flashManager->SetEmitCount(1);
  flashManager->SetColor({1.0f, 0.75f, 0.25f, 1.0f});
  flashManager->SetLifeTime(0.3f);
  flashManager->SetSpeed(0.0f);
  flashManager->SetUniformScaleRange(8.0f, 8.0f);
  flashManager->SetScaleVelocity(8.0f);

  // camera->SetRotate({ 0.0f,0.0f,0.0f });
  // camera->SetTranslate({ 0.0f,0.0f,0.0f });
  // Object3dCommon::GetInstance()->SetDefaultCamera(camera);

  debugCameraRotate = camera->GetRotate();
  debugCameraTranslate = camera->GetTranslate();

  // jsonファイルのデシリアライズ
  // jsonファイルのパス名
  const std::string fullpath = std::string("resources/") + "scene.json";

  // ファイルストリーム
  std::ifstream file;

  // ファイルを開く
  file.open(fullpath);
  // ファイルオープン失敗をチェック
  if (file.fail()) {
    assert(0);
  }

  nlohmann::json deserialized;

  // ファイルから読み込みメモリへ格納
  file >> deserialized;

  // 正しいレベルデータかチェック
  assert(deserialized.is_object());
  assert(deserialized.contains("name"));
  assert(deserialized["name"].is_string());

  // レベルデータを構造体に格納していく
  LevelData levelData;

  //"name"を文字列として取得
  levelData.name = deserialized["name"].get<std::string>();
  assert(levelData.name == "scene");

  //"object"の全オブジェクトを走査
  for (nlohmann::json &object : deserialized["objects"]) {
    // オブジェクトを1つ分の妥当性のチェック
    assert(object.contains("type"));

    if (object["type"].get<std::string>() == "MESH") {
      // 1個分の要素の準備
      levelData.objects.emplace_back(ObjectData{});
      ObjectData &objectData = levelData.objects.back();
      objectData.type = object["type"].get<std::string>();
      objectData.name = object["name"].get<std::string>();

      // トランスフォームのパラメーター読み込み
      nlohmann::json &transform = object["transform"];
      // 平行移動"transform"
      objectData.transform.translation.x = (float)transform["translation"][0];
      objectData.transform.translation.y = (float)transform["translation"][2];
      objectData.transform.translation.z = (float)transform["translation"][1];
      // 回転角"rotation"
      objectData.transform.rotation.x = -(float)transform["rotation"][0];
      objectData.transform.rotation.y = -(float)transform["rotation"][2];
      objectData.transform.rotation.z = -(float)transform["rotation"][1];
      // 拡大縮小"scaling"
      objectData.transform.scaling.x = (float)transform["scaling"][0];
      objectData.transform.scaling.y = (float)transform["scaling"][2];
      objectData.transform.scaling.z = (float)transform["scaling"][1];

      //"file_name"
      if (object.contains("file_name")) {
        objectData.file_name = object["file_name"].get<std::string>();
      }
    }
  }

  constexpr float kDegreeToRadian = 3.14159265f / 180.0f;
  for (const ObjectData &objectData : levelData.objects) {
    if (objectData.file_name.empty()) {
      continue;
    }

    ModelManager::GetInstance()->LoadModel(objectData.file_name);
    Model *levelModel =
        ModelManager::GetInstance()->FindModel(objectData.file_name);
    assert(levelModel != nullptr);

    auto levelObject = std::make_unique<Object3d>();
    levelObject->Initialize(Object3dCommon::GetInstance());
    levelObject->SetModel(levelModel);
    levelObject->SetCamera(camera);
    levelObject->SetTranslate(objectData.transform.translation);
    levelObject->SetRotate({
        objectData.transform.rotation.x * kDegreeToRadian,
        objectData.transform.rotation.y * kDegreeToRadian,
        objectData.transform.rotation.z * kDegreeToRadian,
    });
    levelObject->SetScale(objectData.transform.scaling);
    levelObjects.emplace_back(std::move(levelObject));
  }
}

void GamePlayScene::Update() {
  if (input->TriggerKey(DIK_ESCAPE)) {
    sceneManager->ChangeScene(SceneType::Title);
    return;
  }

#ifdef USE_IMGUI
  static bool showDemoWindow = false;
  ImGui::Begin("ImGui Test");
  ImGui::Text("ImGui is working!");
  ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
  ImGui::Checkbox("Show Demo Window", &showDemoWindow);
  ImGui::End();

  if (showDemoWindow) {
    ImGui::ShowDemoWindow(&showDemoWindow);
  }
#endif

  if (input->PushMouse(1)) {
    debugCameraRotate.x +=
        static_cast<float>(input->GetMouseMoveY()) * kMouseRotateSensitivity;
    debugCameraRotate.y +=
        static_cast<float>(input->GetMouseMoveX()) * kMouseRotateSensitivity;
    debugCameraRotate.x = std::clamp(debugCameraRotate.x, -1.45f, 1.45f);
  }

  if (input->GetMouseWheel() != 0) {
    debugCameraTranslate.z +=
        static_cast<float>(input->GetMouseWheel()) * kMouseWheelSensitivity;
  }

  camera->SetRotate(debugCameraRotate);
  camera->SetTranslate(debugCameraTranslate);

  if (input->TriggerKey(DIK_0)) {
    OutputDebugStringA("Hit 0\n");
  }

  if (input->TriggerKey(DIK_SPACE)) {
    particleManager->Emit({0.0f, 0.0f, 0.0f});
    particleManager2->Emit({0.0f, 0.0f, 0.0f});
    smokeManager->Emit({0.0f, -1.7f, 0.0f});
    flashManager->Emit({0.0f, 0.0f, 0.0f});
    flashLightTime = 0.15f;
    // std::string message = "Particle count: " +
    // std::to_string(particleManager->GetParticleCount()) + "\n";
    // OutputDebugStringA(message.c_str());
  }

  particleManager->Update(1.0f / 60.0f);
  particleManager2->Update(1.0f / 60.0f);
  smokeManager->Update(1.0f / 60.0f);
  flashManager->Update(1.0f / 60.0f);
  const float flashLightIntensity = 8.0f * (flashLightTime / 0.15f);
  object3d2->SetPointLight({0.0f, 0.0f, 0.0f}, {1.0f, 0.75f, 0.25f, 1.0f},
                           flashLightIntensity, 14.0f, 2.0f);
  if (flashLightTime > 0.0f) {
    flashLightTime -= 1.0f / 60.0f;
    if (flashLightTime < 0.0f) {
      flashLightTime = 0.0f;
    }
  }

  camera->Update();
  keyframeAnimation->Update(1.0f / 60.0f);
  keyframeAnimation->ApplyAnimation(skeleton, keyframeAnimation->GetAnimation(),
                                    keyframeAnimation->GetAnimationTime());
  model->Update(skeleton);
  model->Update(skinCluster, skeleton);
  object3d->Update();
  object3d2->Update();
  for (const std::unique_ptr<Object3d> &levelObject : levelObjects) {
    levelObject->Update();
  }
}

void GamePlayScene::Draw() {
  /*SpriteCommon::GetInstance()->CreatePrimitiveTopology();
  sprite->Update();
  sprite->Draw();*/

  /*skyBox->Update(camera);
  skyBox->CreatePrimitiveTopology();
  skyBox->Draw();*/

  /*ring->Update(camera);
  ring->CreatePrimitiveTopology();
  ring->Draw();*/

  /*cylinder->Update(camera);
  cylinder->CreatePrimitiveTopology();
  cylinder->Draw();*/

  Object3dCommon::GetInstance()->CreateSkinningPrimitiveTopology();
  object3d->Draw(skinCluster);

  Object3dCommon::GetInstance()->CreatePrimitiveTopology();
  object3d2->Draw();
  for (const std::unique_ptr<Object3d> &levelObject : levelObjects) {
    levelObject->Draw();
  }
  model->DrawSkeleton(skeleton, object3d->GetWorldMatrix(), camera);

  particleManager->Draw();
  particleManager2->Draw();
  flashManager->Draw();
  smokeManager->Draw();
}

void GamePlayScene::Finalize() {
  // ゲーム固有リソースだけを解放する。
  // 共通リソースは、この後Framework::FinalizeFramework()が解放する。
  levelObjects.clear();
  delete particleManager;
  delete particleManager2;
  delete smokeManager;
  delete flashManager;
  delete keyframeAnimation;
  delete skyBox;
  delete ring;
  delete cylinder;
  delete object3d;
  delete object3d2;
  delete model;
  delete terrainModel;
  delete sprite;
  delete camera;
}
