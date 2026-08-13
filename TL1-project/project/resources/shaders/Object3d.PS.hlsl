#include "object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct Camera
{
    float3 worldPosition;
};

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float2 padding;
};

ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float4> gTexture : register(t0);
TextureCube<float4> gEnvironmentTexture : register(t1);

SamplerState gSampler : register(s0);

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<PointLight> gPointLight : register(b3);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    output.color = gMaterial.color * textureColor;
     //textureのα値が0.5以下のときにPixelを棄却
    if (textureColor.a <= 0.5f)
    {
        discard;
    }
    //textureのα値が0のときにPixelを棄却
    if (textureColor.a == 0.0f)
    {
        discard;
    }
    //output.colorのα値が0の時にPixelを棄却
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    if (gMaterial.enableLighting != 0) {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
        float RdotE = dot(reflectLight, toEye);
        float specularPow = pow(saturate(RdotE), 16.0f);
        
        float3 CameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
        float3 reflectedVector = reflect(CameraToPosition, normalize(input.normal));
        float3 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector).rgb;
        
        //拡散反射
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        //鏡面反射
         float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        //拡散反射+鏡面反射
        //output.color.rgb = diffuse + specular;
        //output.color.rgb += environmentColor; //環境光を加算
        //α値
        output.color.a = gMaterial.color.a * textureColor.a;
        
        float3 baseColor = gMaterial.color.rgb * textureColor.rgb;
        float3 directionalDiffuse = baseColor * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;

        float3 pointDirection = gPointLight.position - input.worldPosition;
        float pointDistance = length(pointDirection);
        pointDirection = pointDistance > 0.0001f ? pointDirection / pointDistance : float3(0.0f, 1.0f, 0.0f);
        float pointNdotL = saturate(dot(normalize(input.normal), pointDirection));
        float attenuation = pow(saturate(1.0f - pointDistance / max(gPointLight.radius, 0.0001f)), gPointLight.decay);
        float3 pointDiffuse = baseColor * gPointLight.color.rgb * pointNdotL * gPointLight.intensity * attenuation;

        output.color.rgb = directionalDiffuse + pointDiffuse;
        output.color.a = gMaterial.color.a * textureColor.a;
    } else {
        output.color = gMaterial.color * textureColor;
    }
    
    return output;
}
