#pragma once

#include <vector>
#include <wrl.h>
#include <d3d12.h>
#include <numbers>
#include <cmath>

#include "Vector.h"
#include "Matrix.h"
#include "MathFunc.h"
#include "Camera.h"
#include "DirectXCommon.h"
#include "TextureManager.h"

class Ring {
public:
    struct Transform {
        Vector3 scale{ 1.0f, 1.0f, 1.0f };
        Vector3 rotate{ 0.0f, 0.0f, 0.0f };
        Vector3 translate{ 0.0f, 0.0f, 0.0f };
    };

    struct VertexData {
        Vector4 position;
        Vector2 texcoord;
    };

    struct TransformationMatrix {
        Matrix4x4 WVP;
        Matrix4x4 World;
    };

    void Initialize(DirectXCommon* dxCommon);
    void Update(Camera* camera);
    void Draw();

    //共通描画設定
    void CreatePrimitiveTopology();

private:
    
    const uint32_t kRingDivide = 32;
    const float kOuterRadius = 1.0f;
    const float kInnerRadius = 0.2;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

    //ルートシグネチャの作成
    void CreateRootSignature();
    //グラフィックスパイプラインの生成
    void GenerateGraphicsPipeLine();

    DirectXCommon* dxCommon_ = nullptr;
    TextureManager* textureManager = nullptr;

    std::vector<VertexData> vertices_;

    //バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};


    VertexData* vertexData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
    TransformationMatrix* wvpData_ = nullptr;

    Matrix4x4 worldMatrix_;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
    Microsoft::WRL::ComPtr <ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    //マテリアルデータ
    //バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
    //バッファリソース内のデータを指すポインタ
    //Material* materialData = nullptr;

    //座標変換行列
    //バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
    //バッファリソース内のデータを指すポインタ
    TransformationMatrix* transformationMatrixData = nullptr;

    Transform transform;
    Transform cameraTransform;

    Matrix4x4 worldMatrix = MakeIdentity4x4();
    Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(Window::kClientWidth) / static_cast<float>(Window::kClientHeight), 0.1f, 100.0f);
    Matrix4x4 viewProjectionMatrix;
    Matrix4x4 worldViewProjectionMatrix;

};

