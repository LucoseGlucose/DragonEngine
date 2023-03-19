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
    float p_roughness = .5f;
    float p_metallic = .5f;
    float p_normalStrength = 1.f;
    float p_aoStrength = 1.f;
    float3 p_emissive = float3(0.f, 0.f, 0.f);
}

struct Light
{
    float3 color;
    float type;
    float3 position;
    float pad0;
    float3 direction;
    float pad1;
    float2 falloff;
    float2 radius;
};

cbuffer WorldParameters : register(b2)
{
    float3 p_cameraPosition;
    Light p_lights[5];
    float3 p_ambientColor = float3(1.f, 1.f, 1.f);
}

Texture2D<float4> t_albedoW : register(t0);
Texture2D<float4> t_roughnessW : register(t1);
Texture2D<float4> t_metallicW : register(t2);
Texture2D<float4> t_normalN : register(t3);
Texture2D<float4> t_AOW : register(t4);
Texture2D<float4> t_EmissiveW : register(t5);

TextureCube<float4> t_irradiance : register(t6);
TextureCube<float4> t_specularReflections : register(t7);
Texture2D<float4> t_brdfLUTL : register(t8);

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

uint querySpecularTextureLevels()
{
    uint width, height, levels;
    t_specularReflections.GetDimensions(0, width, height, levels);
    return levels;
}

void CalcLightData(in PS_INPUT input, in Light light, out float3 radiance, out float3 dir)
{
    if (light.type == 1)
    {
        float3 toLight = light.position - input.worldPosition;
        dir = normalize(toLight);
        
        float distance = length(toLight);
        float attenuation = 1.f / (1 + light.falloff.x * distance + light.falloff.y * (distance * distance));
        radiance = light.color * attenuation;
    }
    if (light.type == 2)
    {
        dir = -normalize(light.direction);
        radiance = light.color;
    }
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 albedo = t_albedoW.Sample(s_sampler, input.uv) * p_albedo;
    float alpha = albedo.a;
    
    float3 albedoColor = albedo.rgb;
    
    float metallic = t_metallicW.Sample(s_sampler, input.uv).r * p_metallic;
    float roughness = t_roughnessW.Sample(s_sampler, input.uv).r * p_roughness;

    float3 normal = normalize(t_normalN.Sample(s_sampler, input.uv).xyz * 2.f - 1.f);
    normal.x *= -p_normalStrength;
    normal.y *= p_normalStrength;
    normal = normalize(mul(normal, input.tangentMatrix));
    
    float3 viewDirection = normalize(p_cameraPosition - input.worldPosition);

    float surfaceViewAngle = max(0.f, dot(normal, viewDirection));
    float3 specularReflectionVector = 2.f * surfaceViewAngle * normal - viewDirection;
    
    float3 fresnelReflectance = lerp(Fdielectric, albedoColor, metallic);
    float3 directLighting = 0.f;
    
    for (int i = 0; i < 5; i++)
    {
        Light light = p_lights[i];
        if (light.type == 0) continue;

        float3 radiance;
        float3 lightDir;
        CalcLightData(input, light, radiance, lightDir);
        
        float3 halfVector = normalize(lightDir + viewDirection);
        
        float surfaceLightAngle = max(0.f, dot(normal, lightDir));
        float surfaceHalfAngle = max(0.f, dot(normal, halfVector));

        float3 fresnel = fresnelSchlick(fresnelReflectance, max(0.f, dot(halfVector, viewDirection)));
        float normalDistribution = ndfGGX(surfaceHalfAngle, roughness);
        float geometricAttenuation = gaSchlickGGX(surfaceLightAngle, surfaceViewAngle, roughness);
        
        float energyConservation = lerp(float3(1.f, 1.f, 1.f) - fresnel, float3(0.f, 0.f, 0.f), metallic);
        
        float3 diffuseBRDF = energyConservation * albedoColor;
        float3 specularBRDF = (fresnel * normalDistribution * geometricAttenuation)
            / max(Epsilon, 4.f * surfaceLightAngle * surfaceViewAngle);
        
        directLighting += (diffuseBRDF + specularBRDF) * radiance * surfaceLightAngle;
    }
    
    float3 ambientLighting = 0.f;
    float3 irradiance = t_irradiance.Sample(s_sampler, normal).rgb;

    float3 fresnel = fresnelSchlick(fresnelReflectance, surfaceViewAngle);
    float3 kd = lerp(1.f - fresnel, 0.f, metallic);
    float3 diffuseIBL = kd * albedoColor * irradiance;
    
    uint specularTextureLevels = querySpecularTextureLevels();
    float3 specularIrradiance = t_specularReflections.SampleLevel(s_sampler, specularReflectionVector,
        roughness * specularTextureLevels).rgb;
    
    float2 specularBRDF = t_brdfLUTL.SampleLevel(s_sampler, float2(surfaceViewAngle, roughness), 0.f).rg;
    
    float3 specularIBL = (fresnelReflectance * specularBRDF.x + specularBRDF.y * (1.f - roughness)) * specularIrradiance;

    ambientLighting = diffuseIBL + specularIBL;
    ambientLighting *= p_ambientColor;
    
    float ao = t_AOW.Sample(s_sampler, input.uv).r * p_aoStrength;
    ambientLighting *= ao;
    
    float3 emissive = t_EmissiveW.Sample(s_sampler, input.uv).rgb * p_emissive;
    
    return float4(directLighting + ambientLighting + emissive, alpha);
}