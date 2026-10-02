#include <Windows.h>

#include <d3d12.h>
#include <dxgidebug.h>

#include "Framework.h"

#include "AudioManager.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "ModelCommon.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "Window.h"

#pragma comment(lib, "dxguid.lib")

Framework::Framework() = default;

Framework::~Framework() = default;

int Framework::Run() {
  InitializeFramework();
  Initialize();

  while (!ProcessMessage()) {
    UpdateFramework();
    DrawFramework();
  }

  Finalize();
  FinalizeFramework();
  return 0;
}

void Framework::InitializeFramework() {
#ifdef _DEBUG
  // デバイス生成前にDirectX 12のデバッグレイヤーを有効化する。
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
    debugController->EnableDebugLayer();
    debugController->SetEnableGPUBasedValidation(TRUE);
  }
#endif

  window = new Window();
  dxCommon = new DirectXCommon();
  srvManager = new SrvManager();
  imguiManager = new ImGuiManager();
  audioManager = new AudioManager();
  input = new Input();
  modelCommon = new ModelCommon();

  window->Initialize();
  dxCommon->Initialize(window);
  srvManager->Initialize(dxCommon);
  imguiManager->Initialize(window, dxCommon, srvManager);
  input->Initialize(window);
  audioManager->Initialize();

  TextureManager::GetInstance()->Initialize(dxCommon, srvManager);
  ModelManager::GetInstance()->Initialize(dxCommon);
  SpriteCommon::GetInstance()->Initialize(dxCommon);
  modelCommon->Initialize(dxCommon);
  Object3dCommon::GetInstance()->Initialize(dxCommon);

#ifdef _DEBUG
  ID3D12InfoQueue *infoQueue = nullptr;
  if (SUCCEEDED(
          dxCommon->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

    D3D12_MESSAGE_ID denyIds[] = {
        D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE};
    D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};
    D3D12_INFO_QUEUE_FILTER filter{};
    filter.DenyList.NumIDs = _countof(denyIds);
    filter.DenyList.pIDList = denyIds;
    filter.DenyList.NumSeverities = _countof(severities);
    filter.DenyList.pSeverityList = severities;
    infoQueue->PushStorageFilter(&filter);
    infoQueue->Release();
  }
#endif
}

bool Framework::ProcessMessage() { return window->ProcessMessage(); }

void Framework::UpdateFramework() {
  input->Update();
  imguiManager->Begin();

  // 仮想関数なので、実体がGameならGame::Update()が呼ばれる。
  Update();

  imguiManager->End();
}

void Framework::DrawFramework() {
  srvManager->PreDraw();
  dxCommon->PreDraw();

  // 仮想関数なので、実体がGameならGame::Draw()が呼ばれる。
  Draw();

  imguiManager->Draw();
  dxCommon->PostDraw();
}

void Framework::FinalizeFramework() {
  imguiManager->Finalize();
  audioManager->Finalize();

  ModelManager::GetInstance()->Finalize();
  TextureManager::GetInstance()->Finalize();

  Object3dCommon::GetInstance()->Finalize();
  delete modelCommon;
  SpriteCommon::GetInstance()->Finalize();
  delete input;
  delete imguiManager;
  delete audioManager;
  delete srvManager;

  window->Finalize();
  delete dxCommon;
  delete window;

#ifdef _DEBUG
  if (debugController) {
    debugController->Release();
    debugController = nullptr;
  }
#endif

  IDXGIDebug1 *debug = nullptr;
  if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
    debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
    debug->Release();
  }
}
