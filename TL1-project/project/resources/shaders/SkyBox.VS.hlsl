#include "SkyBox.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};

cbuffer TransformationMatrix : register(b0) {
    float4x4 WVP;
    float4x4 World;
};

//ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput {
    float4 position : POSITION;
};

VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;
    output.position = mul(input.position, WVP).xyww;
    output.texcoord = input.position.xyz;
    return output;
}
