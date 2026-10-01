#pragma once

#include "Camera.h"
#include "DirectXCommon.h"
#include <wrl.h>

class Object3dCommon {
public:
  static Object3dCommon *GetInstance();
  void Finalize();

  Object3dCommon(const Object3dCommon &) = delete;
  Object3dCommon &operator=(const Object3dCommon &) = delete;

  void Initialize(DirectXCommon *dxCommon);

  // 共通描画設定
  void CreatePrimitiveTopology();
  void CreateSkinningPrimitiveTopology();
  // void PreDraw();

  // セッター
  void SetDefaultCamera(Camera *camera) { this->defaultCamera_ = camera; }

  // ゲッター
  DirectXCommon *GetDxCommon() { return dxCommon_; }
  Camera *GetDefaultCamera() const { return defaultCamera_; }

private:
  Object3dCommon() = default;
  ~Object3dCommon() = default;

  static Object3dCommon *instance;

  // ルートシグネチャの作成
  void CreateRootSignature();
  void CreateSkinningRootSignature();
  // グラフィックスパイプラインの生成
  void CreateGraphicsPipelineState();
  void CreateSkinningGraphicsPipelineState();

  DirectXCommon *dxCommon_;
  Camera *defaultCamera_ = nullptr;

  D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
  Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
  Microsoft::WRL::ComPtr<ID3D12RootSignature> skinningRootSignature = nullptr;
  Microsoft::WRL::ComPtr<ID3D12PipelineState> skinningGraphicsPipelineState =
      nullptr;
  Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
  Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
};
