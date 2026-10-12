#pragma pack_matrix(row_major)

#include "Shared/LightDefinitions.h"

#if !defined(LIGHTING_MODEL_GOURAUD) && !defined(LIGHTING_MODEL_LAMBERT) && !defined(LIGHTING_MODEL_PHONG) && !defined(LIGHTING_MODEL_UNLIT)
    #define LIGHTING_MODEL_PHONG 1
#endif

cbuffer Viewconstants : register(b0)
{
    matrix VP;
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
    uint PointLightCount = 0;
    uint SpotLightCount = 0;
    uint2 LightPadding;
};

float4 CalculateAmbientLight(FAmbientLightInfo info)
{
    float4 result;
    return result;
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
    float4 texColor = g_txColor.Sample(g_Sample, input.uv + UVOffset);
    float4 albedo = texColor * BaseColor;
    float3 totalLight = float3(0, 0, 0);
    float3 N = normalize(input.normal);
   
    //PointLight
    for (uint i = 0; i < PointLightCount; ++i)
    {
        FPointLightInfo PointLight = PointLights[i];
        float4 PositionRadius = PointLight.PositionRadius;
        float4 ColorIntensity = PointLight.ColorIntensity;
        float4 FalloffEnabled = PointLight.FalloffEnabled;
        
        float distance = length(PositionRadius.xyz - input.worldPos);
        float radius = max(PositionRadius.w, 0.001f);
        
        float Basefalloff = saturate(1.0f - distance / radius);
        float falloff = pow(Basefalloff, max(FalloffEnabled.x, 0.001f));
        
        float enabled = FalloffEnabled.y;
        float intensity = ColorIntensity.w;
        
        float3 lightDir = normalize(PositionRadius.xyz - input.worldPos);
        
        float pointNdotL = saturate(dot(N, lightDir));
        float3 pointLight = ColorIntensity.rgb * pointNdotL * falloff * enabled * intensity;
        totalLight += pointLight;
    }
    
    //SpotLight
    for (uint j = 0; j < SpotLightCount; ++j)
    {
        FSpotLightInfo SpotLight = SpotLights[j];
        float4 PositionRadius = SpotLight.PositionRadius;
        float4 ColorIntensity = SpotLight.ColorIntensity;
        float4 FalloffEnabled = SpotLight.FalloffEnabled;
        float4 OutAngleInvCosDiff = SpotLight.OutAngleInvCosDiff;
        float3 SpotFowardVector = normalize(SpotLight.FowardVector.xyz);

        float distance = length(PositionRadius.xyz - input.worldPos);
        float radius = max(PositionRadius.w, 0.001f);

        //SpotLight는 거리에 따른 감쇠와 Cone에 따른 감쇠 두가지가 있다.
        float Distancefalloff = saturate(1.0f - distance / radius);
        
        //콘의 어느 범위에 속하는가 = cos(=spot 전방벡터와 -L 내적) - cosOuter * (1 / cosInner - cosOuter)
        float3 lightDir = normalize(PositionRadius.xyz - input.worldPos);
        float spotforwarddotL = dot(-lightDir, SpotFowardVector);
        float Conefalloff = saturate((spotforwarddotL - OutAngleInvCosDiff.x) * OutAngleInvCosDiff.y);
        float falloff = pow(Distancefalloff, max(FalloffEnabled.x, 0.001f)) * Conefalloff;
        
        float enabled = FalloffEnabled.y;
        float intensity = ColorIntensity.w;
        
        float spotNdotL = saturate(dot(N, lightDir));
        float3 spotLight = ColorIntensity.rgb * spotNdotL * falloff * enabled * intensity;
        totalLight += spotLight;

    }
    
    //Directional Light, AmbientLight
    float directionNdotL = saturate(dot(N, -DirectionalLight.Direction.xyz));
    float3 lighting = AmbientLight.ColorIntensity.xyz * AmbientLight.ColorIntensity.w
          + DirectionalLight.ColorIntensity.xyz * DirectionalLight.ColorIntensity.w
          * directionNdotL;
    
    // Opaque는 알파를 1로 고정한다. 뷰포트 RT를 ImGui가 알파 블렌딩으로 그리므로 알파가 남으면 비쳐 보인다
    float alpha = bOpaque > 0.5f ? 1.0f : albedo.a;
    
    float3 finalRGB = albedo.rgb * (lighting + totalLight);
    return float4(finalRGB, alpha);
}
