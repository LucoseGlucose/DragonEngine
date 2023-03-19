struct VS_INPUT
{
    float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float3 tangent : TANGENT;
	float3 bitangent : BINORMAL;
};

struct VS_OUTPUT
{
    float4 screenPosition : SV_Position;
    float2 uv : TEXCOORD;
    float3 worldPosition : POSITION;
    float3x3 tangentMatrix : TMATRIX;
    float3 geometryNormal : NORMAL;
};

cbuffer VertexParameters : register(b0)
{
    float4x4 p_mvpMat = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    float4x4 p_modelMat = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.screenPosition = mul(float4(input.position, 1.f), p_mvpMat);

    output.uv = input.uv;
    output.worldPosition = mul(float4(input.position, 1.f), p_modelMat).xyz;

    float3 t = normalize(mul(float4(input.tangent, 0.f), p_modelMat).xyz);
    float3 b = normalize(mul(float4(input.bitangent, 0.f), p_modelMat).xyz);
    float3 n = normalize(mul(float4(input.normal, 0.f), p_modelMat).xyz);
    
    output.tangentMatrix = float3x3(t, b, n);
    output.geometryNormal = mul(input.normal, (float3x3)p_modelMat);
    
    return output;
}