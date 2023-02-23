struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D<float4> t : register(t0);
SamplerState s : register(s0);

cbuffer Params : register(b0)
{
    uint currentMip = 0;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    return t.SampleLevel(s, input.uv, currentMip - 1);
}