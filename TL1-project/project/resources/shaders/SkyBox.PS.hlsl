#include "SkyBox.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    static const float PI = 3.14159265358979323846f;
    float3 direction = normalize(input.texcoord);
    float2 uv;
    uv.x = atan2(direction.z, direction.x) / (2.0f * PI) + 0.5f;
    uv.y = 0.5f - asin(clamp(direction.y, -1.0f, 1.0f)) / PI;

    float4 textureColor = gTexture.Sample(gSampler, uv);
    output.color = textureColor;
    
    return output;
}
