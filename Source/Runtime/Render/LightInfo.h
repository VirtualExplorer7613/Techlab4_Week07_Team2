#pragma once
#include "Math/Vector4.h"

constexpr uint32 MaxPointLights = 16;
constexpr uint32 MaxSpotLights = 16;

struct alignas(16) FAmbientLightInfo
{
	FVector4 Color;
 };

struct alignas(16) FDirectionalLightInfo
{
	FVector4 Direction;
	FVector4 Color;
};

struct alignas(16) FPointLightInfo
{
	FVector4 PositionRadius;
	FVector4 ColorFalloff;
};

struct alignas(16) FSpotLightInfo
{
	FVector4 PositionRadius;
	FVector4 Direction;
	FVector4 ConeCos;
	FVector4 ColorFalloff;
};

struct alignas(16) FLightConstants
{
	FAmbientLightInfo Ambient;
	FDirectionalLightInfo Directional;
	FPointLightInfo PointLights[MaxPointLights];
	FSpotLightInfo SpotLights[MaxSpotLights];
	uint32 PointLightCount = 0;
	uint32 SpotLightCount = 0;
	uint32 Padding[2] = { 0, 0 };
};