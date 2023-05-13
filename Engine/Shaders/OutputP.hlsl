struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D<float4> t_inputTexture : register(t0);
SamplerState s_sampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 sceneColor = t_inputTexture.Sample(s_sampler, input.uv).rgb;

    return float4(sceneColor, 1);
}