#include "EnginePCH.h"
#include "PointLightComponent.h"
#include "Engine/World.h"
#include "Render/LineBatcher.h"

#include <cmath>

namespace
{
    void AddWireCircle(
        FLineBatcher* LineBatcher,
        const FVector& Center,
        const FVector& AxisA,
        const FVector& AxisB,
        float Radius,
        const FVector4& Color)
    {
        constexpr int32 SegmentCount = 32;

        FVector Previous = Center + AxisA * Radius;

        for (int32 i = 1; i <= SegmentCount; ++i)
        {
            const float Angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(SegmentCount);

            const FVector Current = Center + AxisA * (std::cos(Angle) * Radius) + AxisB * (std::sin(Angle) * Radius);

            LineBatcher->AddLine(Previous, Current, Color);
            Previous = Current;
        }
    }
}

UPointLightComponent::~UPointLightComponent()
{
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().UnregisterPointLight(this);
        }
    }
}

void UPointLightComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().RegisterPointLight(this);
        }
    }
}

void UPointLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
    if (!LineBatcher)
        return;

    const float Radius = GetAttenuationRadius();

    if (Radius <= 0.0f)
        return;

    const FVector Center = GetWorldLocation();
    const FVector4& Color = GetLightColor();

    const FVector X(1.0f, 0.0f, 0.0f);
    const FVector Y(0.0f, 1.0f, 0.0f);
    const FVector Z(0.0f, 0.0f, 1.0f);

    // 서로 직교하는 원 세 개로 구형 범위를 표시한다.
    AddWireCircle(LineBatcher, Center, X, Y, Radius, Color);
    AddWireCircle(LineBatcher, Center, X, Z, Radius, Color);
    AddWireCircle(LineBatcher, Center, Y, Z, Radius, Color);

    // 중심에서 표면까지 실제 반경을 보여주는 선.
    LineBatcher->AddLine(Center, Center + X * Radius, Color);
}