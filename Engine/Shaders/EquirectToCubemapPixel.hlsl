struct PS_INPUT
{
    float4 position : SV_Position;
    float4 localPosition : COLOR;
};

Texture2D<float4> t_texture : register(t0);
SamplerState s_sampler : register(s0);

static const float PI = 3.141592;
static const float TwoPI = 2 * PI;

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 v = normalize(input.localPosition.xyz);
    
    float phi = atan2(v.z, v.x);
    float theta = acos(v.y);
    
    return t_texture.Sample(s_sampler, float2(phi / TwoPI, theta / PI));
}