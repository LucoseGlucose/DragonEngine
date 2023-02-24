struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D<float4> t_texture : register(t0);
SamplerState s_sampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    return t_texture.Sample(s_sampler, input.uv);
}