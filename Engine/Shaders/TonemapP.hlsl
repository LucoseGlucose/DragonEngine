struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D<float4> t_sceneTexture : register(t0);
SamplerState s_sampler : register(s0);

float3 ToneMapACESFilmic(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 sceneColor = t_sceneTexture.Sample(s_sampler, input.uv).rgb;
    float3 toneMapped = ToneMapACESFilmic(sceneColor);

    return float4(toneMapped, 1);
}