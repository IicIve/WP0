#include "Game.h"

#include "SceneManager.h"

Game::Game() = default;

Game::~Game() = default;

void Game::Initialize() {
  sceneManager =
      std::make_unique<SceneManager>(input, dxCommon, srvManager, modelCommon);
  sceneManager->Initialize(SceneType::Title);
}

void Game::Update() { sceneManager->Update(); }

void Game::Draw() { sceneManager->Draw(); }

void Game::Finalize() {
  if (sceneManager) {
    sceneManager->Finalize();
    sceneManager.reset();
  }
}
