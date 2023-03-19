struct PS_INPUT
{
    float4 position : SV_Position;
    float3 localPosition : POSITION;
};

TextureCube<float4> t_texture : register(t0);
SamplerState s_sampler : register(s0);

cbuffer Params : register(b0)
{
    float p_currentMip = 0;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    return t_texture.SampleLevel(s_sampler, input.localPosition, p_currentMip - 1);
}