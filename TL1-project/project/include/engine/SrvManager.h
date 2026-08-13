#pragma once

#include "DirectXCommon.h"

#include <wrl.h>

class SrvManager {
public:
	void Initialize(DirectXCommon* dxCommon);

	//ヒープセットコマンド
	void PreDraw();

	//デスクリプタハンドル計算
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	uint32_t Allocate();

	void CreateSRVForTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT MipLevels);
	void CreateSRVForStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
private:
	//関数

	//SRVセットコマンド
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	//SRV生成(テクスチャ用)
	//SRV生成(structuredBuffer用)

	//変数

	DirectXCommon* dxCommon = nullptr;

	//最大SRV数 (最大テクスチャ数)
	static const uint32_t kMaxSrvCount;
	//SRV用のデスクリプタサイズ
	uint32_t descriptorSize;
	//SRV用のデスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	//次に使用するSRVインデックス
	uint32_t useIndex = 0;


};

