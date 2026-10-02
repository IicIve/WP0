#pragma once

class AudioManager;
class DirectXCommon;
class ImGuiManager;
class Input;
class ModelCommon;
class SrvManager;
class Window;
struct ID3D12Debug1;

class Framework {
public:
  Framework();
  virtual ~Framework();

  Framework(const Framework &) = delete;
  Framework &operator=(const Framework &) = delete;

  int Run();

protected:
  // 派生クラス固有の処理。Run()から自動的に呼び出される。
  virtual void Initialize() = 0;
  virtual void Update() = 0;
  virtual void Draw() = 0;
  virtual void Finalize() = 0;

  Input *input = nullptr;
  Window *window = nullptr;
  DirectXCommon *dxCommon = nullptr;
  SrvManager *srvManager = nullptr;
  ImGuiManager *imguiManager = nullptr;
  AudioManager *audioManager = nullptr;
  ModelCommon *modelCommon = nullptr;

private:
  void InitializeFramework();
  void UpdateFramework();
  void DrawFramework();
  void FinalizeFramework();
  bool ProcessMessage();

  ID3D12Debug1 *debugController = nullptr;
};
