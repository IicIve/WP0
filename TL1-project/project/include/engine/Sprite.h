#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <string>

#include "Vector.h"
#include "Matrix.h"
#include "MathFunc.h"
#include "Window.h"
#include "TextureManager.h"

class SpriteCommon;
using namespace Vector;
using namespace Matrix;

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

//頂点データ
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//マテリアルデータ
struct Material {
	Vector4 color;
	int enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

//座標変換行列データ
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

class Sprite {
public:
	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);
	void Update();
	void Draw();

	//getter
	const Vector2& GetPosition() const { return position; }
	float GetRotation() const { return rotation; }
	const Vector2& GetSize() const { return size; }
	const Vector4& GetColor() const { return materialData->color; }
	const Vector2& GetAnchorPoint() const { return anchorPoint; }
	bool GetIsFlipX() const { return isFlipX_; }
	bool GetIsFlipY() const { return isFlipY_; }
	const Vector2& GetTextureLeftTop() const { return textureLeftTop; }
	const Vector2& GetTextureSize() const { return textureSize; }

	//setter
	void SetPosition(const Vector2& position) { this->position = position; }
	void SetRotation(float rotation) { this->rotation = rotation; }
	void SetSize(const Vector2& size) { this->size = size; }
	void SetColor(const Vector4& color) { materialData->color = color; }
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }
	void SetIsFlipX(bool isFlipX) { this->isFlipX_ = isFlipX; }
	void SetIsFlipY(bool isFlipY) { this->isFlipY_ = isFlipY; }
	void SetTextureLeftTop(const Vector2& leftTop) { this->textureLeftTop = leftTop; }
	void SetTextureSize(const Vector2& size) { this->textureSize = size; }

private:
	//Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	Transform transform{ {1.0f,1.0f,1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	//スプライトの変換行列
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(Window::kClientWidth), static_cast<float>(Window::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

	void AdjustTextureSize();

	SpriteCommon* spriteCommon = nullptr;
	TextureManager* textureManager = nullptr;

	//spriteのposition
	Vector2 position = { 0.0f, 0.0f };

	//spriteのrotation
	float rotation = 0.0f;

	//spriteのsize
	Vector2 size = { 640.0f, 360.0f };
	
	//アンカーポイント
	Vector2 anchorPoint = { 0.0f, 0.0f };

	//左右フリップ
	bool isFlipX_ = false;
	bool isFlipY_ = false;

	//テクスチャ左上座標
	Vector2 textureLeftTop = { 0.0f,0.0f };
	//テクスチャ切り出しサイズ
	Vector2 textureSize = { 100.0f, 100.0f };

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	//バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	std::uint32_t* indexData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	//マテリアル用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;
	

	
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	//Matrix4x4* transformationMatrixData = nullptr;
	//transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	TransformationMatrix* transformationMatrixData = nullptr;

	
	TransformationMatrix* wvpData = nullptr;

	//
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};

	//テクスチャ番号
	std::string textureFilePath_;
	//uint32_t textureIndex = UINT32_MAX;
	//uint32_t textureIndex = 0;

};

