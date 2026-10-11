#pragma pack_matrix(row_major)

#define LIGHTING_MODEL_LAMBERT 0
#define LIGHTING_MODEL_PHONG 1
#define LIGHTING_MODEL_GOURAUD 2
#define LIGHTING_MODEL_BLINN_PHONG 3

#ifndef LIGHTING_MODEL
#define LIGHTING_MODEL LIGHTING_MODEL_LAMBERT
#endif

static const float PI = 3.14159265358979323846f;
static const float Shininess = 64.0f;
static const float3 SpecularColor = float3(0.04f, 0.04f, 0.04f);

cbuffer Viewconstants : register(b0)
{
    matrix VP;
    float4 CameraPos;
};

cbuffer Worldconstants : register(b2)
{
    matrix World;
};

cbuffer MaterialParams : register(b1)
{
    float4 BaseColor;
    float2 UVOffset;
    float bOpaque;
    float Padding;
};

struct FireBallLight
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 FalloffEnabled;
};

cbuffer FireBallLightConstants : register(b3)
{
    FireBallLight FireBalls[16];
    uint FireBallCount;
    float3 FireBallLightPadding;
};

struct VS_INPUT
{
    float3 p : POSITION; // Input position from vertex buffer
    float3 n : NORMAL;
    float4 c : COLOR; // Input color from vertex buffer
    float2 t : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

Texture2D g_txColor : register(t0);
SamplerState g_Sample : register(s0);

// 임시 하드코딩 Directional Light. 빛이 "향하는" 방향이다.
static const float3 LightDir = normalize(float3(0.5f, 0.5f, -1.0f));
static const float3 LightColor = float3(1.0f, 1.0f, 1.0f);
static const float3 AmbientColor = float3(0.1f, 0.1f, 0.1f);

void ComputeLight(float3 N, float3 L, float3 V, float3 Albedo, float3 LightColor, out float3 OutDiffuse, out float3 OutSpecular)
{
    float NdotL = saturate(dot(N, L));
    float3 E = LightColor * PI;
    
    float3 Kd = Albedo * (1.0f - SpecularColor);
    OutDiffuse = Kd / PI * NdotL * E;
    
#if LIGHTING_MODEL == LIGHTING_MODEL_PHONG
    if(NdotL <= 0.0f)
    {
        OutSpecular = float3(0, 0, 0);
        return;
    }
    float3 R = reflect(-L, N);
    float RdotV = saturate(dot(R, V));
    float Norm = (Shininess + 2.0f) / (2.0f * PI);
    OutSpecular = SpecularColor * Norm * pow(RdotV, Shininess) * E * NdotL;
#elif LIGHTING_MODEL == LIGHTING_MODEL_LAMBERT
    OutDiffuse = Albedo / PI * NdotL * E;
    OutSpecular = float3(0,0,0);
#else
    OutSpecular = float3(0, 0, 0);
#endif
}

float3 ACESFilmicToneMapping(float3 color)
{
    // ACES Filmic Tone Mapping
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    output.worldPos = mul(float4(input.p, 1.0f), World).xyz;
    output.position = mul(mul(float4(input.p, 1.0f), World), VP);
    
    // w=0으로 이동 성분을 빼고 월드 공간으로 보낸다. 비균등 스케일이면 역전치가 필요하다.
    output.normal = mul(float4(input.n, 0.0f), World).xyz;
    output.color = input.c;
    output.uv = input.t;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Mesh 표면색
    float4 texColor = g_txColor.Sample(g_Sample, input.uv + UVOffset);
    float4 albedo = texColor * BaseColor;
    albedo.rgb = pow(albedo.rgb, 2.2f);
    float3 N = normalize(input.normal); // 현재 표면이 향하고 있는 방향
    float3 V = normalize(CameraPos.xyz - input.worldPos); // 현재 표면에서 카메라가 있는 방향
    
    float3 totalDiffuse = 0, totalSpecular = 0;
    float3 d, s;
    
    ComputeLight(N, -LightDir, V, albedo.rgb, LightColor, d, s);
    totalDiffuse += d;
    totalSpecular += s;
   
    for (uint i = 0; i < FireBallCount; ++i)
    {
        FireBallLight fireBall = FireBalls[i];
        float4 PositionRadius = fireBall.PositionRadius;
        float4 ColorIntensity = fireBall.ColorIntensity;
        float4 FalloffEnabled = fireBall.FalloffEnabled;
        
        float distance = length(PositionRadius.xyz - input.worldPos);
        float radius = max(PositionRadius.w, 0.001f);
        
        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(FalloffEnabled.x, 0.001f));
        
        float enabled = FalloffEnabled.y;
        float intensity = ColorIntensity.w;
        
        float3 lightDir = normalize(PositionRadius.xyz - input.worldPos);
        ComputeLight(N, lightDir, V, albedo.rgb, ColorIntensity.rgb * intensity * enabled * falloff, d, s);
        totalDiffuse += d;
        totalSpecular += s;
    }
    
    
    // Opaque는 알파를 1로 고정한다. 뷰포트 RT를 ImGui가 알파 블렌딩으로 그리므로 알파가 남으면 비쳐 보인다
    float alpha = bOpaque > 0.5f ? 1.0f : albedo.a;
    
    float3 finalRGB = albedo.rgb * AmbientColor + totalDiffuse + totalSpecular;
    finalRGB = ACESFilmicToneMapping(finalRGB);
    finalRGB = pow(finalRGB, 1.0f / 2.2f); // Gamma Correction
    return float4(finalRGB, alpha);
}

