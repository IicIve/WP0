#include <Windows.h>
#include <memory>

#include "Framework.h"
#include "Game.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { // Framework型でGameを扱い、仮想関数を通してゲーム固有処理を呼び出す。
  std::unique_ptr<Framework> game = std::make_unique<Game>();
  return game->Run();
}