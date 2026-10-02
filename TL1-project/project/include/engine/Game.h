#pragma once

#include "Framework.h"
#include <memory>

class SceneManager;

class Game final : public Framework {
public:
  Game();
  ~Game() override;

protected:
  void Initialize() override;
  void Update() override;
  void Draw() override;
  void Finalize() override;

private:
  std::unique_ptr<SceneManager> sceneManager;
};
