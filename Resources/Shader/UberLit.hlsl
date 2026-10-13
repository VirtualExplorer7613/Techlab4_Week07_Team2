#pragma pack_matrix(row_major)

#include "Shared/LightDefinitions.h"

#if !defined(LIGHTING_MODEL_GOURAUD) && !defined(LIGHTING_MODEL_LAMBERT) && !defined(LIGHTING_MODEL_PHONG) && !defined(LIGHTING_MODEL_UNLIT) && !defined(VIEWMODE_WORLDNORMAL)
    #define LIGHTING_MODEL_PHONG 1
#endif

static const float PI = 3.14159265359f;
static const float3 Ks = float3(0.04f, 0.04f, 0.04f);

cbuffer Viewconstants : register(b0)
{
    matrix VP;
    float3 CameraLocation;
    float ViewPadding;
};

cbuffer Worldconstants : register(b2)
{
    matrix World;
    matrix WorldInvTranspose;
};

cbuffer MaterialParams : register(b1)
{
    float4 BaseColor;
    float2 UVOffset;
    float bOpaque;
    float Shininess;
};

struct FPointLightInfo
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 FalloffEnabled;
};

struct FSpotLightInfo
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 FalloffEnabled;
    float4 OutAngleInvCosDiff;
    float4 FowardVector;
};

struct FAmbientLightInfo
{
    float4 ColorIntensity;
};

struct FDirectionalLightInfo
{
    float4 ColorIntensity;
    float4 Direction;
};

cbuffer LightConstants : register(b3)
{
    FPointLightInfo PointLights[NUM_POINT_LIGHT];
    FSpotLightInfo SpotLights[NUM_SPOT_LIGHT];
    FAmbientLightInfo AmbientLight;
    FDirectionalLightInfo DirectionalLight;
    uint PointLightCount;
    uint SpotLightCount;
    uint2 LightPadding;
};

cbuffer TileBasedCullingConstants : register(b4)
{
    matrix View;
    matrix InvProjection;
    
    uint2 TileCount;
    uint2 ScreenSize;
    
    uint TilePointLightCount;
    uint TileSpotLightCount;
    uint2 Padding;
};

// ACES Filmic Tone Mapping
float3 ACESFilmicToneMapping(float3 color)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

// 라이트 하나가 표면에 주는 빛. Diffuse는 Albedo와 곱하고 Specular는 그 뒤에 더한다.
struct FLightResult
{
    float3 Diffuse;
    float3 Specular;
};

// 라이트 하나의 Blinn-Phong 계산. Lambert 모드도 같은 함수를 쓰고 Specular만 버린다.
// N: 표면 노멀, L: 표면→라이트, V: 표면→카메라 (모두 정규화), Radiance: 색 × 세기 × 감쇠
FLightResult BlinnPhong(float3 N, float3 L, float3 V, float3 Radiance)
{
    FLightResult Result;

    float NdotL = saturate(dot(N, L));
    float n = max(Shininess, 1.0f);
    Result.Diffuse = Radiance * NdotL * 1/PI * (1.0f - Ks);
    float norm = (n + 8.0f) / (8.0f * PI); // Blinn-Phong 정규화 상수
    float3 H = normalize(L + V);
    float NdotH = saturate(dot(N, H));
    // 빛을 받지 못하는 면(N·L <= 0)에는 하이라이트가 생기지 않게 한다.
    Result.Specular = Radiance * pow(NdotH, n) * norm * NdotL * Ks;

    return Result;
}

FLightResult CalculatePointLight(FPointLightInfo info, float3 worldPos, float3 normal, float3 viewDir)
{
    float4 PositionRadius = info.PositionRadius;
    float4 ColorIntensity = info.ColorIntensity;
    float4 FalloffEnabled = info.FalloffEnabled;

    float3 toLight = PositionRadius.xyz - worldPos;
    float distance = length(toLight);
    float3 lightDir = toLight / max(distance, 0.0001f); // length를 한 번만 계산하고 0으로 나누기를 막는다

    float radius = max(PositionRadius.w, 0.001f);
    float Basefalloff = saturate(1.0f - distance / radius);
    float falloff = pow(Basefalloff, max(FalloffEnabled.x, 0.001f));

    float enabled = FalloffEnabled.y;
    float3 radiance = ColorIntensity.rgb * ColorIntensity.w * falloff * enabled;

    return BlinnPhong(normal, lightDir, viewDir, radiance);
}

FLightResult CalculateSpotLight(FSpotLightInfo info, float3 worldPos, float3 normal, float3 viewDir)
{
    float4 PositionRadius = info.PositionRadius;
    float4 ColorIntensity = info.ColorIntensity;
    float4 FalloffEnabled = info.FalloffEnabled;
    float4 OutAngleInvCosDiff = info.OutAngleInvCosDiff;
    float3 SpotFowardVector = normalize(info.FowardVector.xyz);

    float3 toLight = PositionRadius.xyz - worldPos;
    float distance = length(toLight);
    float3 lightDir = toLight / max(distance, 0.0001f); // length를 한 번만 계산하고 0으로 나누기를 막는다

    float radius = max(PositionRadius.w, 0.001f);

    //SpotLight는 거리에 따른 감쇠와 Cone에 따른 감쇠 두가지가 있다.
    float Distancefalloff = saturate(1.0f - distance / radius);

    //콘의 어느 범위에 속하는가 = (cos(=spot 전방벡터와 -L 내적) - cosOuter) * (1 / (cosInner - cosOuter))
    //콘 감쇠 지수(FalloffEnabled.z)는 거리 감쇠 지수(FalloffEnabled.x)와 독립적이다 (D3D9 Falloff)
    float spotforwarddotL = saturate(dot(-lightDir, SpotFowardVector));
    float Conefalloff = pow(saturate((spotforwarddotL - OutAngleInvCosDiff.x) * OutAngleInvCosDiff.y), max(FalloffEnabled.z, 0.001f));
    float falloff = pow(Distancefalloff, max(FalloffEnabled.x, 0.001f)) * Conefalloff;

    float enabled = FalloffEnabled.y;
    float3 radiance = ColorIntensity.rgb * ColorIntensity.w * falloff * enabled;

    return BlinnPhong(normal, lightDir, viewDir, radiance);
}

float3 CalculateAmbientLight(FAmbientLightInfo info)
{
    return info.ColorIntensity.xyz * info.ColorIntensity.w;
}

FLightResult CalculateDirectionalLight(FDirectionalLightInfo info, float3 normal, float3 viewDir)
{
    // Direction은 빛이 나아가는 방향이므로 뒤집어야 표면→라이트 방향이 된다.
    float3 lightDir = normalize(-info.Direction.xyz);
    float3 radiance = info.ColorIntensity.xyz * info.ColorIntensity.w;

    return BlinnPhong(normal, lightDir, viewDir, radiance);
}

// 직접광(Point·Spot·Directional)의 Diffuse/Specular 합. Ambient는 따로 더한다.
FLightResult CalculateLighting(float3 worldPos, float3 normal, float3 viewDir)
{
    FLightResult Total;
    Total.Diffuse = float3(0, 0, 0);
    Total.Specular = float3(0, 0, 0);

    //TODO: TileBasedCulling이 완성되면 픽셀이 포함된 타일을 찾아 라이트를 순회하도록 변경
    
    //PointLight
    for (uint i = 0; i < PointLightCount; ++i)
    {
        FLightResult Light = CalculatePointLight(PointLights[i], worldPos, normal, viewDir);
        Total.Diffuse += Light.Diffuse;
        Total.Specular += Light.Specular;
    }

    //SpotLight
    for (uint j = 0; j < SpotLightCount; ++j)
    {
        FLightResult Light = CalculateSpotLight(SpotLights[j], worldPos, normal, viewDir);
        Total.Diffuse += Light.Diffuse;
        Total.Specular += Light.Specular;
    }

    //Directional Light
    FLightResult Directional = CalculateDirectionalLight(DirectionalLight, normal, viewDir);
    Total.Diffuse += Directional.Diffuse;
    Total.Specular += Directional.Specular;

    return Total;
}


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
#if LIGHTING_MODEL_GOURAUD
    float3 Diffuse : TEXCOORD2;
    float3 Specular : TEXCOORD3;
#endif
};

Texture2D g_txColor : register(t0);
SamplerState g_Sample : register(s0);

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    output.worldPos = mul(float4(input.p, 1.0f), World).xyz;
    output.position = mul(mul(float4(input.p, 1.0f), World), VP);

    // 비균등 스케일에서도 올바른 노멀값을 구하기 위해 역전치행렬 사용
    output.normal = normalize(mul(float4(input.n, 0.0f), WorldInvTranspose).xyz);
    
#if LIGHTING_MODEL_GOURAUD
    float3 viewDir = normalize(CameraLocation - output.worldPos);
    FLightResult Light = CalculateLighting(output.worldPos, output.normal, viewDir);
    output.Diffuse = Light.Diffuse;
    output.Specular = Light.Specular;
#endif
    output.color = input.c;
    output.uv = input.t;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_txColor.Sample(g_Sample, input.uv + UVOffset);
    float4 albedo = texColor * BaseColor;
    

    float3 normal = normalize(input.normal);

    // Opaque는 알파를 1로 고정한다. 뷰포트 RT를 ImGui가 알파 블렌딩으로 그리므로 알파가 남으면 비쳐 보인다
    float alpha = bOpaque > 0.5f ? 1.0f : albedo.a;

#if LIGHTING_MODEL_UNLIT
    return float4(albedo.xyz, 1.0f);

#elif VIEWMODE_WORLDNORMAL
    return float4(normal * 0.5f + 0.5f, 1.0f);   // -1~1 노멀을 0~1 색으로
#endif
   
    albedo.rgb = pow(albedo.rgb, 2.2f);
    float3 viewDir = normalize(CameraLocation - input.worldPos);   // 표면 → 카메라
    float3 ambient = CalculateAmbientLight(AmbientLight);
    
    float3 finalRGB = albedo.rgb;
    
#if LIGHTING_MODEL_GOURAUD
    finalRGB = finalRGB * (ambient + input.Diffuse) + input.Specular;
    finalRGB = ACESFilmicToneMapping(finalRGB);
    finalRGB = pow(finalRGB, 1.0f / 2.2f);
    return float4(finalRGB, alpha);
#endif

FLightResult Light = CalculateLighting(input.worldPos, normal, viewDir);
    
#if LIGHTING_MODEL_PHONG
    finalRGB =  finalRGB * (ambient + Light.Diffuse) + Light.Specular;
 #else
    finalRGB = finalRGB * (ambient + Light.Diffuse);
#endif
    finalRGB = ACESFilmicToneMapping(finalRGB);
    finalRGB = pow(finalRGB, 1.0f / 2.2f);
    return float4(finalRGB, alpha);
}
