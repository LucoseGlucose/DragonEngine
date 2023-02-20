struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D<float4> t : register(t0);
SamplerState s : register(s0);

cbuffer PixelParams : register(b1)
{
    float4 color = float4(1, 1, 1, 1);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    return t.Sample(s, input.uv) * color;
}