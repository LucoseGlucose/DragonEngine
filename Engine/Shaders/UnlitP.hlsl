struct PS_INPUT
{
    float4 screenPosition : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer SurfaceParameters : register(b1)
{
    float4 p_albedo;
    float p_alphaClip;
}

Texture2D<float4> t_albedoW : register(t0);

SamplerState s_sampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 albedo = t_albedoW.Sample(s_sampler, input.uv) * p_albedo;

    if (albedo.a <= p_alphaClip) discard;

    return albedo;
}