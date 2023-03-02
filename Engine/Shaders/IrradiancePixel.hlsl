struct PS_INPUT
{
    float4 position : SV_Position;
    float3 localPosition : POSITION;
};

static const float PI = 3.141592;
static const float TwoPI = 2 * PI;
static const float Epsilon = 0.00001;

TextureCube t_skybox : register(t0);
SamplerState s_sampler : register(s0);

cbuffer Samples : register(b0)
{
    int sampleCount = 1;
}

float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}
float2 sampleHammersley(uint i, float invSampleCount)
{
    return float2(i * invSampleCount, radicalInverse_VdC(i));
}

float3 sampleHemisphere(float u1, float u2)
{
    const float u1p = sqrt(max(0.0, 1.0 - u1 * u1));
    return float3(cos(TwoPI * u2) * u1p, sin(TwoPI * u2) * u1p, u1);
}

void computeBasisVectors(const float3 N, out float3 S, out float3 T)
{
    T = cross(N, float3(0.0, 1.0, 0.0));
    T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

    T = normalize(T);
    S = normalize(cross(N, T));
}

float3 tangentToWorld(const float3 v, const float3 N, const float3 S, const float3 T)
{
    return S * v.x + T * v.y + N * v.z;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float invSampleCount = 1.0 / float(sampleCount);
    float3 N = normalize(input.localPosition);
	
    float3 S, T;
    computeBasisVectors(N, S, T);
    
    float3 irradiance = 0.0;
    for (uint i = 0; i < sampleCount; ++i)
    {
        float2 u = sampleHammersley(i, invSampleCount);
        float3 Li = tangentToWorld(sampleHemisphere(u.x, u.y), N, S, T);
        float cosTheta = max(0.0, dot(Li, N));
        
        irradiance += 2.0 * t_skybox.Sample(s_sampler, Li).rgb * cosTheta;
    }
    irradiance /= float(sampleCount);

    return float4(irradiance, 1.0);
}