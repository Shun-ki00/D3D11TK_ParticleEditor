
cbuffer ParticleConstantBuffer : register(b0)
{
    matrix m_worldMatrix : packoffset(c0);
    matrix m_viewMatrix : packoffset(c4);
    matrix m_projectionMatrix : packoffset(c8);
    matrix m_billboardMatrix : packoffset(c12);
    float4 f_time : packoffset(c16);
};

struct VS_INPUT
{
    float4 positionOS : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 positionCS : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};