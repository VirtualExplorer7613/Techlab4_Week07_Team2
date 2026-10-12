#pragma pack_matrix(row_major)

#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

#define LIGHTING_MODEL_LAMBERT 0
#define LIGHTING_MODEL_PHONG 1
#define LIGHTING_MODEL_GOURAUD 2
#define LIGHTING_MODEL_BLINN_PHONG 3
#define LIGHTING_MODEL_UNLIT 4
#define LIGHTING_MODEL_WORLD_NORMAL 5

#ifndef LIGHTING_MODEL
#define LIGHTING_MODEL LIGHTING_MODEL_PHONG
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

struct FAmbientLightInfo
{
    float4 Color;
};

struct FDirectionalLightInfo
{
    float4 Direction;
    float4 Color;
};

struct FPointLightInfo
{
    float4 PositionRadius;
    float4 ColorFalloff;
};

struct FSpotLightInfo
{
    float4 PositionRadius;
    float4 Direction;
    float4 ConeCos;
    float4 ColorFalloff;
};

cbuffer LightConstants : register(b4)
{
    FAmbientLightInfo AmbientLight;
    FDirectionalLightInfo DirectionalLight;
    FPointLightInfo PointLights[MAX_POINT_LIGHTS];
    FSpotLightInfo SpotLights[MAX_SPOT_LIGHTS];
    uint PointLightCount;
    uint SpotLightCount;
    float2 LightPadding;
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
#if LIGHTING_MODEL == LIGHTING_MODEL_GOURAUD
    float3 diffuse : TEXCOORD2;
    float3 specular : TEXCOORD3;
#endif
};

Texture2D g_txColor : register(t0);
SamplerState g_Sample : register(s0);


void ComputeLight(float3 N, float3 L, float3 V, float3 Albedo, float3 LightColor, out float3 OutDiffuse, out float3 OutSpecular)
{
    float NdotL = saturate(dot(N, L));
    float3 E = LightColor * PI;
    
    float3 Kd = Albedo * (1.0f - SpecularColor);
    OutDiffuse = Kd / PI * NdotL * E;
    
#if LIGHTING_MODEL == LIGHTING_MODEL_PHONG || LIGHTING_MODEL == LIGHTING_MODEL_GOURAUD
    if(NdotL <= 0.0f) { OutSpecular = float3(0, 0, 0); return; }
    float3 R = reflect(-L, N);
    float RdotV = saturate(dot(R, V));
    float Norm = (Shininess + 2.0f) / (2.0f * PI);
    OutSpecular = SpecularColor * Norm * pow(RdotV, Shininess) * E * NdotL;
#elif LIGHTING_MODEL == LIGHTING_MODEL_LAMBERT
    OutDiffuse = Albedo / PI * NdotL * E;
    OutSpecular = float3(0,0,0);
#elif LIGHTING_MODEL == LIGHTING_MODEL_BLINN_PHONG
    if(NdotL <= 0.0f) { OutSpecular = float3(0,0,0); return; }
    float3 H = normalize(L + V);
    float NdotH = saturate(dot(H, N));
    float Norm = (Shininess + 8.0f) / (8.0f * PI);
    OutSpecular = SpecularColor * Norm * pow(NdotH, Shininess) * E * NdotL;

#else
    OutSpecular = float3(0, 0, 0);
#endif
}

void AccumlateLight(float3 N, float3 V, float3 WorldPos, float3 Albedo, inout float3 OutDiffuse, inout float3 OutSpecular)
{
    OutDiffuse = float3(0, 0, 0);
    OutSpecular = float3(0, 0, 0);
    float3 d, s;
    ComputeLight(N, -DirectionalLight.Direction.xyz, V, Albedo, DirectionalLight.Color.rgb, d, s);
    OutDiffuse += d;
    OutSpecular += s; 
    
    for (uint i = 0; i < FireBallCount; ++i)
    {
        FireBallLight fireBall = FireBalls[i];
        float4 PositionRadius = fireBall.PositionRadius;
        float4 ColorIntensity = fireBall.ColorIntensity;
        float4 FalloffEnabled = fireBall.FalloffEnabled;
        
        float distance = length(PositionRadius.xyz - WorldPos);
        float radius = max(PositionRadius.w, 0.001f);
        
        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(FalloffEnabled.x, 0.001f));
        
        float enabled = FalloffEnabled.y;
        float intensity = ColorIntensity.w;
        
        float3 lightDir = normalize(PositionRadius.xyz - WorldPos);
        ComputeLight(N, lightDir, V, Albedo, ColorIntensity.rgb * intensity * enabled * falloff, d, s);
        OutDiffuse += d;
        OutSpecular += s;
    }
    
    for (uint p = 0; p < PointLightCount; ++p)
    {
        FPointLightInfo pointLight = PointLights[p];
        float4 PositionRadius = pointLight.PositionRadius;
        float4 ColorFalloff = pointLight.ColorFalloff;
        
        float distance = length(PositionRadius.xyz - WorldPos);
        float radius = max(PositionRadius.w, 0.001f);
        
        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(ColorFalloff.w, 0.001f));
        
        float3 lightDir = normalize(PositionRadius.xyz - WorldPos);
        ComputeLight(N, lightDir, V, Albedo, ColorFalloff.rgb * falloff, d, s);
        OutDiffuse += d;
        OutSpecular += s;
    }
   
    
    for (uint sp = 0; sp < SpotLightCount; ++sp)                        
    {
        FSpotLightInfo spotLight = SpotLights[sp]; 
        float4 PositionRadius = spotLight.PositionRadius;
        float4 ColorFalloff = spotLight.ColorFalloff;
        float3 SpotDir = spotLight.Direction.xyz;
        float CosInner = spotLight.ConeCos.x; 
        float CosOuter = spotLight.ConeCos.y; 

        // 거리 감쇠 (Point와 동일)
        float distance = length(PositionRadius.xyz - WorldPos);
        float radius = max(PositionRadius.w, 0.001f);

        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(ColorFalloff.w, 0.001f));

        float3 lightDir = normalize(PositionRadius.xyz - WorldPos);

        // 원뿔 감쇠
        float CosAngle = dot(-lightDir, SpotDir);
        float Cone = saturate((CosAngle - CosOuter) / max(CosInner - CosOuter, 0.0001f));

        ComputeLight(N, lightDir, V, Albedo, ColorFalloff.rgb * falloff * Cone, d, s); // ★ * Cone
        OutDiffuse += d;
        OutSpecular += s;
    }

}

float3 ACESFilmicToneMapping(float3 color)
{
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
#if LIGHTING_MODEL == LIGHTING_MODEL_GOURAUD
    float3 N = normalize(output.normal);
    float3 V = normalize(CameraPos.xyz - output.worldPos);
    AccumlateLight(N, V, output.worldPos, float3(1,1,1), output.diffuse, output.specular);
#endif
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Mesh 표면색
    float4 texColor = g_txColor.Sample(g_Sample, input.uv + UVOffset);
    float4 albedo = texColor * BaseColor;

    // Opaque는 알파를 1로 고정한다. 뷰포트 RT를 ImGui가 알파 블렌딩으로 그리므로 알파가 남으면 비쳐 보인다
    float alpha = bOpaque > 0.5f ? 1.0f : albedo.a;

    // ── 덩어리 1: 라이팅이 없는 모드는 여기서 끝 ──
#if LIGHTING_MODEL == LIGHTING_MODEL_UNLIT
    return float4(albedo.rgb, alpha);
#elif LIGHTING_MODEL == LIGHTING_MODEL_WORLD_NORMAL
    return float4(normalize(input.normal) * 0.5f + 0.5f, 1.0f);
#endif

    // ── 덩어리 2: 라이팅 모드 공통 ──
    albedo.rgb = pow(albedo.rgb, 2.2f);
    float3 totalDiffuse = 0, totalSpecular = 0;

#if LIGHTING_MODEL == LIGHTING_MODEL_GOURAUD
    totalDiffuse = albedo.rgb * input.diffuse;
    totalSpecular = input.specular;
#else
    float3 N = normalize(input.normal); // 현재 표면이 향하고 있는 방향
    float3 V = normalize(CameraPos.xyz - input.worldPos); // 현재 표면에서 카메라가 있는 방향
    AccumlateLight(N, V, input.worldPos, albedo.rgb, totalDiffuse, totalSpecular);
#endif

    float3 finalRGB = albedo.rgb * AmbientLight.Color.rgb + totalDiffuse + totalSpecular;
    finalRGB = ACESFilmicToneMapping(finalRGB);
    finalRGB = pow(finalRGB, 1.0f / 2.2f);
    return float4(finalRGB, alpha);
}
