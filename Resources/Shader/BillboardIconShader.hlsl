#pragma pack_matrix(row_major)
// 에디터 아이콘 빌보드. 흰색 아이콘 텍스처에 Tint(라이트 색)를 곱해 그린다.
cbuffer constants : register(b0)
{
    matrix VP;
};

cbuffer Worldconstants : register(b2)
{
    matrix World;
};

cbuffer IconParams : register(b1)
{
    float4 Tint;
};

Texture2D IconTexture : register(t0);
SamplerState IconSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(mul(float4(input.position, 1.0f), World), VP);
    output.uv = input.uv;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 color = IconTexture.Sample(IconSampler, input.uv);
    color.rgb *= Tint.rgb;
    return color;
}
