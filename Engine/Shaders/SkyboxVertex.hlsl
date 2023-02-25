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
    float4 position : SV_Position;
    float4 skyboxPosition : COLOR;
};

cbuffer VertexParams : register(b0)
{
    float4x4 p_mvpMat = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = mul(float4(input.position, 1.0f), p_mvpMat).xyww;
    output.skyboxPosition = float4(input.position, 1.0f);
    
    return output;
}