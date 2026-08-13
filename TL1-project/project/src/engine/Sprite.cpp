#include <Sprite.h>
#include "SpriteCommon.h"

//using namespace MathFunc;

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath) {
	this->spriteCommon = spriteCommon;
	textureManager = TextureManager::GetInstance();

	//Sprite用の頂点リソースを作る
	vertexResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 6);
	//vertexBufferViewを作成
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//spriteのindexResoureを作る
	indexResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	//indexBufferViewを作成
	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	//vertexResourceにデータを書き込む
	vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//スプライトの頂点データの設定
	vertexData[0].position = { 0.0f,1.0f,0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };

	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.0f, 0.0f };

	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };

	vertexData[3].position = { 1.0f, 0.0f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 1.0f, 0.0f };

	/*vertexDataSprite[4].position = { 640.0f, 0.0f,0.0f,1.0f };
	vertexDataSprite[4].texcoord = { 1.0f, 0.0f };
	vertexDataSprite[5].position = { 640.0f, 360.0f,0.0f,1.0f };
	vertexDataSprite[5].texcoord = { 1.0f, 1.0f };*/

	vertexData[0].normal = { 0.0f,0.0f,-1.0f };

	//indexResourceにデータを書き込む
	indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	//インデックスデータの設定
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	////スプライトのマテリアル用のリソース
	materialResource = spriteCommon->GetDxCommon()->CreateBufferResource(256);
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();
	//materialDataSprite->uvTransform = uvTransformMatrix;Upload

	//sprite用のTransformationMatrix用のリソースを作る
	constexpr size_t kCBSizeSprite = (sizeof(Matrix4x4) + 255) & ~255;
    transformationMatrixResource = spriteCommon->GetDxCommon()->CreateBufferResource(kCBSizeSprite);
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
	//constexpr size_t kCBSizeSprite = (sizeof(Matrix4x4) + 255) & ~255;
	//Microsoft::WRL::ComPtr <ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(kCBSizeSprite);
	//transformationMatrixResource = spriteCommon->GetDxCommon()->CreateBufferResource(spriteCommon->GetDxCommon()->GetDevice(), sizeof(Matrix4x4));
	//Matrix4x4* transformationMatrixData = nullptr;
	//transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//*transformationMatrixDataSprite = MakeIdentity4x4();
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{  };
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(Vector4) * 3;

	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	textureManager->LoadTexture(textureFilePath);

	textureFilePath_ = textureFilePath;

	//textureIndex = textureManager->GetSrvIndex(textureFilePath);

	AdjustTextureSize();
}

void Sprite::Update() {

	/*Vector2 velocity = GetPosition();
	velocity.x += 2.0f;
	SetPosition(velocity);*/

	/*float rotate = GetRotation();
	rotate += 0.02f;
	SetRotation(rotate);*/

	/*Vector2 size = GetSize();
	size.x += 0.1f;
	size.y += 0.1f;
	SetSize(size);

	Vector4 color = GetColor();
	color.x += 0.01f;
	if (color.x >= 1.0f) {
		color.x -= 1.0f;
	}
	SetColor(color);*/

	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	//左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}
	//上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

	//アンカーポイントが0,0の時左上が座標
	//アンカーポイントが1,1の時右下が座標
	//アンカーポイントが0.5,0.5の時中心が座標
	vertexData[0].position = { left, bottom, 0.0f, 1.0f };
	vertexData[1].position = { left, top, 0.0f, 1.0f };
	vertexData[2].position = { right, bottom, 0.0f, 1.0f };
	vertexData[3].position = { right, top, 0.0f, 1.0f };

	vertexData[0].texcoord = { tex_left, tex_bottom };
	vertexData[1].texcoord = { tex_left, tex_top };
	vertexData[2].texcoord = { tex_right, tex_bottom };
	vertexData[3].texcoord = { tex_right, tex_top };

	transform.translate = { position.x, position.y, 0.0f };
	transform.rotate = { 0.0f, 0.0f, rotation };
	transform.scale = { size.x, size.y, 1.0f };

	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = MakeIdentity4x4();
	worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	//transformationMatrixData = worldViewProjectionMatrix;
	transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw() {
	//Spriteの描画
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2,TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	spriteCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	spriteCommon->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	spriteCommon->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::AdjustTextureSize() {
	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);
	//画像サイズをテクスチャサイズに合わせる
	size = textureSize;
}
