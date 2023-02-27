struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

SamplerState s_sampler : register(s0);

Texture2D<float4> t_albedoW : register(t0);
Texture2D<float4> t_roughnessW : register(t1);
Texture2D<float4> t_metallicW : register(t2);
Texture2D<float4> t_normalN : register(t3);
Texture2D<float4> t_AOW : register(t4);
Texture2D<float4> t_EmissiveW : register(t5);

cbuffer SurfaceParameters : register(b1)
{
    float4 p_albedo = float4(1.f, 1.f, 1.f, 1.f);
    float p_alphaClip = 0.f;
    float p_roughness = .5f;
    float p_metallic = .5f;
    float p_normalStrength = 1.f;
    float p_aoStrength = 1.f;
    float3 p_emissive = float3(0.f, 0.f, 0.f);
}

struct Light
{
    int type;
    float3 color;
    float2 falloff;
    float3 position;
    float3 direction;
    float2 radius;
};

cbuffer Lights : register(b2)
{
    Light p_lights[5];
}

float4 main(PS_INPUT input) : SV_TARGET
{
    return t_albedoW.Sample(s_sampler, input.uv) * p_albedo;
}