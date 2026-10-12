// UberLit.hlsl
#pragma pack_matrix(row_major)
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

struct PointLight
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 FalloffEnabled;
};

cbuffer PointLightConstants : register(b5)
{
    PointLight PointLights[16];
    uint PointLightCount;
    float3 PointLighttPadding;
};

struct SpotLight
{
    float4 PositionRadius;
    float4 ColorIntensity;
    float4 DirectionCosOuter;
    float4 ConeParams;
};

cbuffer SpotLightConstants : register(b4)
{
    SpotLight SpotLights[16];
    uint SpotLightCount;
    float3 SpotLightPadding;
};

cbuffer CameraConstants : register(b6)
{
    float4 CameraWorldPos; // xyz: 월드 위치, w: 사용하지 않음
};

cbuffer AmbientLightConstants : register(b7)
{
    float4 AmbientLightColor;
};

struct DirectionalLight
{
    float4 ColorIntensity;
    float4 Direction;
};

cbuffer DirectionalLightConstants : register(b8)
{
    DirectionalLight DirectionalLights[16];
    uint DirectionalLightCount;
    float3 DirectionalLightPadding;
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
    float3 specular : TEXCOORD2;
};

Texture2D g_txColor : register(t0);
SamplerState g_Sample : register(s0);

// 기존 화면과 비교하기 위해 기존 고정값을 유지한다.
// LightDir은 빛이 진행하는 방향이다.
// static const float3 LightDir = normalize(float3(0.5f, 0.5f, -1.0f));
// static const float3 LightColor = float3(0.5f, 0.5f, 0.5f);
// static const float3 AmbientColor = float3(0.5f, 0.5f, 0.5f);

static const float Shininess = 10.0f;

float3 CalculateAmbientLight()
{
    return AmbientLightColor.rgb;
}

float3 CalculateDirectionalLight(float3 N)
{
    float3 total = float3(0, 0, 0);

    for (uint i = 0; i < DirectionalLightCount; ++i)
    {
        DirectionalLight light = DirectionalLights[i];

        // 저장된 것은 빛의 진행 방향이다.
        // N·L에는 표면 → 광원 방향이 필요하다.
        float3 L = -light.Direction.xyz;
        float NdotL = saturate(dot(N, L));

        total += light.ColorIntensity.rgb * light.ColorIntensity.w * NdotL;
    }

    return total;
}

float3 CalculatePointLight(PointLight light, float3 worldPos, float3 N)
{
    float3 toLight = light.PositionRadius.xyz - worldPos;
    float distanceToLight = length(toLight);
    float radius = max(light.PositionRadius.w, 0.001f);

    float baseFalloff = saturate(1.0f - distanceToLight / radius);

    float falloff = pow(baseFalloff, max(light.FalloffEnabled.x, 0.001f));

    float3 L = normalize(toLight);
    float NdotL = saturate(dot(N, L));

    float enabled = light.FalloffEnabled.y;
    float intensity = light.ColorIntensity.w;

    return light.ColorIntensity.rgb * NdotL * falloff * enabled * intensity;
}

float3 CalculateSpotLight(SpotLight light, float3 worldPos, float3 normal)
{
    float3 toLight = light.PositionRadius.xyz - worldPos;
    float distanceToLight = length(toLight);
    float radius = light.PositionRadius.w;

    if (distanceToLight <= 0.00001f || distanceToLight >= radius)
        return float3(0, 0, 0);

    // 표면 → 광원 방향
    float3 L = toLight / distanceToLight;

    // 광원의 진행 방향과 광원 -> 표면 방향을 비교
    float3 forward = normalize(light.DirectionCosOuter.xyz);
    float cosTheta = dot(forward, -L);

    float cosOuter = light.DirectionCosOuter.w;
    float cosInner = light.ConeParams.x;
    float coneWidth = cosInner - cosOuter;

    float coneFactor;

    if (coneWidth > 0.000001f)
    {
        coneFactor = saturate((cosTheta - cosOuter) / coneWidth);
    }
    else
    {
        coneFactor = cosTheta >= cosOuter ? 1.0f : 0.0f;
    }

    coneFactor = pow(coneFactor, light.ConeParams.y);

    float distanceFactor = saturate(1.0f - distanceToLight / radius);

    distanceFactor *= distanceFactor;

    float diffuse = saturate(dot(normal, L));

    return light.ColorIntensity.rgb * light.ColorIntensity.w * diffuse * distanceFactor * coneFactor;
}

float3 CalculateDiffuseLighting(float3 worldPos, float3 N)
{
    float3 totalPointLight = float3(0.0f, 0.0f, 0.0f);

    for (uint i = 0; i < PointLightCount; ++i)
    {
        totalPointLight += CalculatePointLight(PointLights[i], worldPos, N);
    }

    float3 totalSpotLight = float3(0.0f, 0.0f, 0.0f);

    for (uint j = 0; j < SpotLightCount; ++j)
    {
        totalSpotLight += CalculateSpotLight(SpotLights[j], worldPos, N);
    }

    float3 lighting = CalculateAmbientLight() + CalculateDirectionalLight(N);

    return lighting + totalPointLight + totalSpotLight;
}

// 광원 종류와 무관한 Blinn-Phong 반사광 계수
float CalculateBlinnPhongSpecular(float3 N, float3 L, float3 V)
{
    // 광원이나 카메라를 등지는 면에는 반사광을 적용하지 않는다.
    if (dot(N, L) <= 0.0f || dot(N, V) <= 0.0f)
        return 0.0f;

    float3 halfVector = L + V;
    float lengthSquared = dot(halfVector, halfVector);

    // 서로 반대인 L과 V의 합은 정규화할 수 없다.
    if (lengthSquared <= 0.000001f)
        return 0.0f;

    float3 H = halfVector * rsqrt(lengthSquared);

    return pow(saturate(dot(N, H)), Shininess);
}

float3 CalculateSpecularLighting(float3 worldPos, float3 N, float3 V)
{
    float3 total = float3(0, 0, 0);
    
    // Directional Light

    for (uint i = 0; i < DirectionalLightCount; ++i)
    {
        DirectionalLight light = DirectionalLights[i];

        // 저장된 진행 방향의 반대가 표면 → 광원 방향이다.
        float3 L = -light.Direction.xyz;

        float specular = CalculateBlinnPhongSpecular(N, L, V);

        total += light.ColorIntensity.rgb * light.ColorIntensity.w * specular;
    }

    // Point Light
    for (uint j = 0; j < PointLightCount; ++j)
    {
        PointLight light = PointLights[j];

        float3 toLight = light.PositionRadius.xyz - worldPos;
        float distanceToLight = length(toLight);
        float radius = max(light.PositionRadius.w, 0.001f);

        if (distanceToLight <= 0.00001f || distanceToLight >= radius)
            continue;

        float3 L = toLight / distanceToLight;
        float specular = CalculateBlinnPhongSpecular(N, L, V);

        // 기존 Point Diffuse와 동일한 거리 감쇠
        float baseFalloff = saturate(1.0f - distanceToLight / radius);
        float falloff = pow(baseFalloff, max(light.FalloffEnabled.x, 0.001f));

        total += light.ColorIntensity.rgb * light.ColorIntensity.w * specular * falloff * light.FalloffEnabled.y;
    }

    // Spot Light
    for (uint k = 0; k < SpotLightCount; ++k)
    {
        SpotLight light = SpotLights[k];

        float3 toLight = light.PositionRadius.xyz - worldPos;
        float distanceToLight = length(toLight);
        float radius = light.PositionRadius.w;

        if (distanceToLight <= 0.00001f || distanceToLight >= radius)
            continue;

        float3 L = toLight / distanceToLight;
        float specular = CalculateBlinnPhongSpecular(N, L, V);

        // 광원의 진행 방향과 광원 → 표면 방향을 비교한다.
        float3 forward = normalize(light.DirectionCosOuter.xyz);
        float cosTheta = dot(forward, -L);

        float cosOuter = light.DirectionCosOuter.w;
        float cosInner = light.ConeParams.x;
        float coneWidth = cosInner - cosOuter;

        // 기존 Spot Diffuse와 동일한 원뿔 감쇠
        float coneFactor;

        if (coneWidth > 0.000001f)
        {
            coneFactor = saturate((cosTheta - cosOuter) / coneWidth);
        }
        else
        {
            coneFactor = cosTheta >= cosOuter ? 1.0f : 0.0f;
        }

        coneFactor = pow(coneFactor, light.ConeParams.y);

        // 기존 Spot Diffuse와 동일한 거리 감쇠
        float distanceFactor = saturate(1.0f - distanceToLight / radius);
        distanceFactor *= distanceFactor;

        total += light.ColorIntensity.rgb * light.ColorIntensity.w * specular * distanceFactor * coneFactor;
    }

    return total;
}

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.worldPos = mul(float4(input.p, 1.0f), World).xyz;
    output.position = mul(mul(float4(input.p, 1.0f), World), VP);
    // w=0으로 이동 성분을 빼고 월드 공간으로 보낸다. 비균등 스케일이면 역전치가 필요하다.
    output.normal = mul(float4(input.n, 0.0f), World).xyz;
    output.uv = input.t;
    
    // GOURAUD만 Vertex 단계에서 빛 계산
#if LIGHTING_MODEL_GOURAUD
    float3 N = normalize(output.normal);
    float3 lighting = CalculateDiffuseLighting(output.worldPos, N);
    output.color = float4(lighting, 1.0f);
    
    float3 V = normalize(CameraWorldPos.xyz - output.worldPos);
    output.specular = CalculateSpecularLighting(output.worldPos, N, V);
#else
    output.color = input.c;
    output.specular = float3(0.0f, 0.0f, 0.0f);
#endif
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_txColor.Sample(g_Sample, input.uv + UVOffset);
    float4 albedo = texColor * BaseColor;
    
    float3 N = normalize(input.normal);
    float3 V = normalize(CameraWorldPos.xyz - input.worldPos);
    float3 diffuse = CalculateDiffuseLighting(input.worldPos, N);
    
#if LIGHTING_MODEL_GOURAUD
    float3 finalRGB = albedo.rgb * input.color.rgb + input.specular;
    float alpha = bOpaque > 0.5f ? 1.0f : albedo.a;

    return float4(finalRGB, alpha);

#elif LIGHTING_MODEL_LAMBERT
    float3 finalRGB = albedo.rgb * diffuse;
    float alpha = bOpaque > 0.5f ? 1.0f : albedo.a;
    
    return float4(finalRGB, alpha);

#elif LIGHTING_MODEL_PHONG
    float3 specular = CalculateSpecularLighting(input.worldPos, N, V);
    
    float3 finalRGB = albedo.rgb * diffuse + specular;
    float alpha = bOpaque > 0.5f ? 1.0f : albedo.a;

    return float4(finalRGB, alpha);

#elif LIGHTING_MODEL_UNLIT
    return albedo;
#else
    return float4(1, 0, 1, 1); // 매크로 누락 확인용 자홍색
#endif
}