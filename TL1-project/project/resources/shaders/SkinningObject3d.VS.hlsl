#include "Object3d.hlsli"

struct Well {
    float4x4 skeletonScaceMatrix;
    float4x4 skeletonSpaceIverseTransposeMatrix;
};

StructuredBuffer<Well> gMatrixPalette : register(t0);

cbuffer TransformationMatrix : register(b0)
{
    float4x4 WVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

//struct DirectionalLight
//{
//    float4 color;
//    float3 direction;
//    float intensity;
//};

struct VertexShaderInput {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 weight : WEIGHT0;
    int4 index : INDEX0;
};

struct Skinned {
    float4 position;
    float3 normal;
};

Skinned Skinning(VertexShaderInput input) {
    Skinned skinned;
    
    //位置の変換
    skinned.position = mul(input.position, gMatrixPalette[input.index.x].skeletonScaceMatrix) * input.weight.x;
    skinned.position += mul(input.position, gMatrixPalette[input.index.y].skeletonScaceMatrix) * input.weight.y;
    skinned.position += mul(input.position, gMatrixPalette[input.index.z].skeletonScaceMatrix) * input.weight.z;
    skinned.position += mul(input.position, gMatrixPalette[input.index.w].skeletonScaceMatrix) * input.weight.w;
    skinned.position.w = 1.0f;//1を確実に入れる
    
    //法線の変換
    skinned.normal = mul(input.normal, (float3x3) gMatrixPalette[input.index.x].skeletonSpaceIverseTransposeMatrix) * input.weight.x;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.y].skeletonSpaceIverseTransposeMatrix) * input.weight.y;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.z].skeletonSpaceIverseTransposeMatrix) * input.weight.z;
    skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[input.index.w].skeletonSpaceIverseTransposeMatrix) * input.weight.w;
    skinned.normal = normalize(skinned.normal);//正規化して戻す
    
    return skinned;
}

VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;
    Skinned skinned = Skinning(input);//まずskinning計算を行う
    output.position = mul(skinned.position, WVP);
    output.worldPosition = mul(skinned.position, World).xyz;
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(skinned.normal, (float3x3) WorldInverseTranspose));
    return output;
}
