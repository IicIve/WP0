#include "Particle.hlsli"

Texture2D<float4> gTexture : register(t1);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);

    float brightness = max(textureColor.r, max(textureColor.g, textureColor.b));
    if (textureColor.a <= 0.01f || brightness <= 0.01f)
    {
        discard;
    }

    output.color = textureColor * input.color;
    output.color.a = textureColor.a * brightness * input.color.a;
    return output;
}
