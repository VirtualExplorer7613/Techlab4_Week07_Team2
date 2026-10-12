#pragma once

#include "Math/Matrix.h"
#include "Math/Vector4.h"
#include "Math/Vector.h"
#include "Math/Frustum.h"
#include "component/LightComponent.h"

class FScene;

// TODO: 라이트 종류별로 클래스 상속 받아서 제공해야 함. (쓸모없는 데이터들을 들고 있는 문제)
class FLightSceneProxy
{
	explicit FLightSceneProxy(ULightComponent* InComponent)
		: Component(InComponent){ }
	~FLightSceneProxy();

	void UpdateTransform();
	void UpdateParameters();

	ULightComponent* GetComponent() { return Component; }
	const FAABB& GetBounds() const { return Bounds; }
	
	const FMatrix& GetLightToWorld() const { return LightToWorld; }
	const FMatrix& GetWorldToLight() const { return WorldToLight; }

	FScene* GetScene() const { return Scene; }
	uint32 GetLightType() const { return static_cast<uint32>(LightType); }

	const FVector& GetPosition() const { return Position; }
	const FVector& GetDirection() const { return Direction; }
	const FVector4& GetColor() const { return LinearColor; }

	float GetIntensity() const { return Intensity; }
	float GetFallOff() const { return FallOff; }
	float GetRadius() const { return Radius; }

	float GetInnerConeCos() const { return InnerConeCos; }
	float GetOuterConeCos() const { return OuterConeCos; }

private:
	friend class FScene;
	FScene* Scene = nullptr;
	ELightType LightType = ELightType::Point;
	ULightComponent* Component;

	FMatrix LightToWorld;
	FMatrix WorldToLight;

	// Light Position
	FVector Position;
	FVector Direction;
	// Light Color
	FVector4 LinearColor;

	float Intensity = 1.0f;
	float Radius = 1000.0f;

	// Type이 Spot일 때만 사용
	float InnerConeCos = 0.0f;
	float OuterConeCos = 0.0f;
	float FallOff = 2.0f;

	// TODO: 컬링용
	FAABB Bounds;

	int32 PackedIndex = INDEX_NONE;
	bool bQueuedForUpdate = false;
	bool bVisible = true;
};