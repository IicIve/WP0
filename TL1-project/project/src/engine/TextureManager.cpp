#include "TextureManager.h"
#include <string.h>
#include <iostream>
#include <filesystem>
#include "Logger.h"
#include "StringUtility.h"

using namespace StringUtility;
using namespace Logger;

TextureManager* TextureManager::instance = nullptr;

uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath) {
	assert(GetDxCommon() != nullptr);

	/*auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);*/
	if (textureDatas.contains(filePath)) {
		return;
	}

	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

	//テクスチャデータを追加
	//textureDatas.resize(textureDatas.size() + 1);
	//追加したテクスチャデータの参照を取得する

	TextureData& textureData = textureDatas[filePath];
	//TextureData& textureData = textureDatas.back();


	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);

	//デバッグ表示
	std::wcout << L"Loading texture: " << filePathW << std::endl;

	// ファイル存在確認
	std::filesystem::path texturePath = std::filesystem::absolute(filePath);
	std::wcout << L"Absolute path: " << texturePath.wstring() << std::endl;
	if (!std::filesystem::exists(texturePath)) {
		std::cerr << "File not found: " << texturePath << std::endl;
		assert(false);
	}
	HRESULT hr;

	if (filePathW.ends_with(L".dds")) {
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = DirectX::LoadFromWICFile(texturePath.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}

	if (FAILED(hr)) {
		std::wcout << L"Failed to load texture: " << filePathW << L", HRESULT = " << std::hex << hr << std::endl;
	}

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	} else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImages);
	}
	assert(SUCCEEDED(hr));

	//textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = GetDxCommon()->CreateTextureResource(textureData.metadata);
	//テクスチャデータの要素番号数をSRVのインデックスとする
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);
	//textureData.srvHandleCPU = GetDxCommon()->GetSRVCPUDescriptorHandle(srvIndex);
	//textureData.srvHandleGPU = GetDxCommon()->GetSRVGPUDescriptorHandle(srvIndex);

	//metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//metadataからcubemapかどうか判断して分岐
	if (textureData.metadata.IsCubemap()) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = UINT_MAX;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	} else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);
	}

	//SRVを作成するDescriptorHeapの場所を決める
	GetDxCommon()->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	//テクスチャデータの転送
	//Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	textureData.intermediateResource = GetDxCommon()->UploadTextureData(textureData.resource, mipImages);

	// コマンドリストを確定
	ID3D12GraphicsCommandList* commandList = GetDxCommon()->GetCommandList();
	commandList->Close();

	// 実行
	ID3D12CommandList* commandLists[] = { commandList };
	GetDxCommon()->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	// GPUの処理完了を待つ
	GetDxCommon()->WaitForGpu();

	// 次の描画に備えてリセット
	GetDxCommon()->GetCommandAllocator()->Reset();
	GetDxCommon()->GetCommandList()->Reset(
		GetDxCommon()->GetCommandAllocator(),
		nullptr
	);
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath) {
	//読み込み済みテクスチャデータ検索
	/*auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);*/
	//if (textureDatas.contains(filePath)){
	//	//読み込み済みなら要素番号を返す
	//	uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
	//	return textureIndex;
	//}
	assert(0);
	return 0;

		assert(textureDatas.contains(filePath));
		return textureDatas[filePath].srvIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	assert(textureDatas.contains(filePath));
	TextureData& textureData = textureDatas[filePath];
	return textureData.srvHandleGPU;
	/*assert(textureDatas.contains(filePath));
	uint32_t index = textureDatas[filePath].srvIndex;
	return srvManager_->GetGPUDescriptorHandle(index);*/
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	assert(textureDatas.contains(filePath));
	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
	/*assert(textureDatas.contains(filePath));
	return textureDatas[filePath].metadata;*/
	// TODO: return ステートメントをここに挿入します
}

