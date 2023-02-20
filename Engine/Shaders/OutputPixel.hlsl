struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

Texture2D<float4> t : register(t0);
SamplerState s : register(s0);

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
    float3 sceneColor = t.Sample(s, input.uv).rgb;
    float3 toneMapped = sceneColor;//ToneMapACESFilmic(sceneColor);
    float3 gammaCorrected = pow(toneMapped, 2.2);

    return float4(gammaCorrected, 1);
}