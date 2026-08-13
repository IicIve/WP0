#pragma once

#include "DirectXCommon.h"
#include "Camera.h"
#include <wrl.h>

class Object3dCommon {
public:
	void Initialize(DirectXCommon* dxCommon);

	//共通描画設定
	void CreatePrimitiveTopology();
	void CreateSkinningPrimitiveTopology();
	//void PreDraw();
	
	//セッター
	void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }

	//ゲッター
	DirectXCommon* GetDxCommon() { return dxCommon_; }
	Camera* GetDefaultCamera() const { return defaultCamera_; }

private:
	//ルートシグネチャの作成
	void CreateRootSignature();
	void CreateSkinningRootSignature();
	//グラフィックスパイプラインの生成
	void CreateGraphicsPipelineState();
	void CreateSkinningGraphicsPipelineState();

	DirectXCommon* dxCommon_;
	Camera* defaultCamera_ = nullptr;

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skinningRootSignature = nullptr;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> skinningGraphicsPipelineState = nullptr;
	Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;


};

