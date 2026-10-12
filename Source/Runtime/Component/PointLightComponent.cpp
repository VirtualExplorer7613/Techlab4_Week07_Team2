#include "EnginePCH.h"
#include "PointLightComponent.h"

#include "Render/LineBatcher.h"

namespace
{
	// 원 하나의 분할 수. 너무 높이면 라인 배처 정점을 빨리 소모한다.
	constexpr int32 CIRCLE_SEGMENT_COUNT = 32;

	// Center를 중심으로 AxisA·AxisB가 이루는 평면에 원을 그린다.
	void AddWireCircle(FLineBatcher* LineBatcher, const FVector& Center, const FVector& AxisA, const FVector& AxisB, float Radius, const FVector4& Color)
	{
		FVector PreviousPoint = Center + AxisA * Radius;
		for (int32 i = 1; i <= CIRCLE_SEGMENT_COUNT; ++i)
		{
			const float Angle = (2.0f * PI * i) / CIRCLE_SEGMENT_COUNT;
			const FVector Point = Center + AxisA * (std::cosf(Angle) * Radius) + AxisB * (std::sinf(Angle) * Radius);
			LineBatcher->AddLine(PreviousPoint, Point, Color);
			PreviousPoint = Point;
		}
	}
}

void UPointLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (LineBatcher == nullptr)
	{
		return;
	}

	const FVector Center = GetWorldLocation();
	const float Radius = (AttenuationRadius > 0.0f) ? AttenuationRadius : 0.0f;

	// 빛이 닿는 범위(구)를 세 평면의 원으로 표시한다
	AddWireCircle(LineBatcher, Center, FVector(1, 0, 0), FVector(0, 1, 0), Radius, LightColor);
	AddWireCircle(LineBatcher, Center, FVector(0, 1, 0), FVector(0, 0, 1), Radius, LightColor);
	AddWireCircle(LineBatcher, Center, FVector(0, 0, 1), FVector(1, 0, 0), Radius, LightColor);
}
