struct PS_INPUT
{
    float4 screenPosition : SV_Position;
    float2 uv : TEXCOORD;
    float3 worldPosition : POSITION;
    float3x3 tangentMatrix : TMATRIX;
    float3 geometryNormal : NORMAL;
};

cbuffer SurfaceParameters : register(b1)
{
    float4 p_albedo = float4(1.f, 1.f, 1.f, 1.f);
    float p_alphaClip = 1.f;
    float p_roughness = .5f;
    float p_metallic = .5f;
    float p_normalStrength = 1.f;
    float p_aoStrength = 1.f;
    float3 p_emissive = float3(0.f, 0.f, 0.f);
}

struct Light
{
    float3 color;
    float pad0;
    float3 position;
    float pad1;
    float3 direction;
    float pad2;
    float2 falloff;
    float2 radius;
};

cbuffer WorldParameters : register(b2)
{
    float3 p_cameraPosition;
    Light p_lights[5];
}

Texture2D<float4> t_albedoW : register(t0);
Texture2D<float4> t_roughnessW : register(t1);
Texture2D<float4> t_metallicW : register(t2);
Texture2D<float4> t_normalN : register(t3);
Texture2D<float4> t_AOW : register(t4);
Texture2D<float4> t_EmissiveW : register(t5);

SamplerState s_sampler : register(s0);

static const float PI = 3.141592;
static const float Epsilon = 0.00001;
static const float3 Fdielectric = 0.04;

float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

float3 fresnelSchlick(float3 F0, float cosTheta)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 albedo = t_albedoW.Sample(s_sampler, input.uv) * p_albedo;
    float alpha = albedo.a;
    
    if (alpha < p_alphaClip) discard;
    float3 albedoColor = albedo.rgb;
    
    float metallic = t_metallicW.Sample(s_sampler, input.uv).r * p_metallic;
    float roughness = t_roughnessW.Sample(s_sampler, input.uv).r * p_roughness;

    float3 normal = normalize(t_normalN.Sample(s_sampler, input.uv).xyz * 2.f - 1.f);
    normal.xy *= p_normalStrength;
    normal = normalize(mul(normal, input.tangentMatrix));
    
    float3 viewDirection = normalize(p_cameraPosition - input.worldPosition);

    float surfaceViewAngle = max(0.f, dot(normal, viewDirection));
    float3 specularReflectionVector = 2.f * surfaceViewAngle * normal - viewDirection;
    
    float3 fresnelReflectance = lerp(Fdielectric, albedoColor, metallic);
    float3 directLighting = 0.f;
    
    for (int i = 0; i < 5; i++)
    {
        Light light = p_lights[i];
        if (!all(light.color)) continue;

        float3 toLight = light.position - input.worldPosition;
        float3 lightDirection = normalize(toLight);
        
        float distance = length(toLight);
        float attenuation = 1.f / (1 + light.falloff.x * distance + light.falloff.y * (distance * distance));
        float3 lightColor = light.color * attenuation;
        
        float3 halfVector = normalize(lightDirection + viewDirection);
        
        float surfaceLightAngle = max(0.f, dot(normal, lightDirection));
        float surfaceHalfAngle = max(0.f, dot(normal, halfVector));

        float3 fresnel = fresnelSchlick(fresnelReflectance, max(0.f, dot(halfVector, viewDirection)));
        float normalDistribution = ndfGGX(surfaceHalfAngle, roughness);
        float geometricAttenuation = gaSchlickGGX(surfaceLightAngle, surfaceViewAngle, roughness);
        
        float energyConservation = lerp(float3(1.f, 1.f, 1.f) - fresnel, float3(0.f, 0.f, 0.f), metallic);
        
        float3 diffuseBRDF = energyConservation * albedoColor;
        float3 specularBRDF = (fresnel * normalDistribution * geometricAttenuation)
            / max(Epsilon, 4.f * surfaceLightAngle * surfaceViewAngle);
        
        directLighting += (diffuseBRDF + specularBRDF) * lightColor * surfaceLightAngle;
    }
    
    return float4(directLighting, alpha);
}