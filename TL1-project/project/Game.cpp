#include "Game.h"

#include "GamePlayScene.h"

Game::Game() = default;

Game::~Game() = default;

void Game::Initialize() {
  gamePlayScene = std::make_unique<GamePlayScene>(input, dxCommon, srvManager, modelCommon);
  gamePlayScene->Initialize();
}

void Game::Update() { gamePlayScene->Update(); }

void Game::Draw() { gamePlayScene->Draw(); }

void Game::Finalize() {
  gamePlayScene->Finalize();
  gamePlayScene.reset();
}
