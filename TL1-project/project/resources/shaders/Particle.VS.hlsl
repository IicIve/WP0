#include "Particle.hlsli"

struct ViewProjection
{
    float4x4 viewProjection;
};

struct ParticleForGPU
{
    float3 translate;
    float scaleX;
    float4 color;
    float rotate;
    float length;
    float2 padding;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

ConstantBuffer<ViewProjection> gViewProjection : register(b0);
StructuredBuffer<ParticleForGPU> gParticles : register(t0);

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    ParticleForGPU particle = gParticles[instanceId];

    float sinRotate = sin(particle.rotate);
    float cosRotate = cos(particle.rotate);
    float2 rotatedPosition = float2(
        input.position.x * cosRotate - input.position.y * sinRotate,
        input.position.x * sinRotate + input.position.y * cosRotate
    );

    float2 scaledPosition = rotatedPosition * float2(particle.scaleX, particle.length);
    float3 worldPosition = float3(scaledPosition, input.position.z) + particle.translate;
    output.position = mul(float4(worldPosition, 1.0f), gViewProjection.viewProjection);
    output.texcoord = input.texcoord;
    output.color = particle.color;

    return output;
}
