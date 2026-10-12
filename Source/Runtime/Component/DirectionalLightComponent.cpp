#include "EnginePCH.h"
#include "DirectionalLightComponent.h"
#include "Engine/World.h"
#include "Render/LineBatcher.h"

UDirectionalLightComponent::~UDirectionalLightComponent()
{
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().UnregisterDirectionalLight(this);
        }
    }
}

void UDirectionalLightComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().RegisterDirectionalLight(this);
        }
    }
}

void UDirectionalLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
    if (!LineBatcher)
        return;

    FTransform WorldTransform;
    WorldTransform.Rotation = GetWorldRotation();

    const FVector Start = GetWorldLocation();
    const FVector Forward = GetLightDirection();
    const FVector Right = WorldTransform.GetRight().GetSafeNormal();
    const FVector Up = WorldTransform.GetUp().GetSafeNormal();

    // 화살표의 에디터 표시 크기. 실제 조명 범위는 아니다.
/*    constexpr float ArrowLength = 2.0f;
    constexpr float HeadLength = 0.5f;
    constexpr float HeadRadius = 0.25f;*/

    constexpr float ArrowLength = 1.5f;   // 기존 2.0f
    constexpr float HeadLength = 0.375f; // 기존 0.5f
    constexpr float HeadRadius = 0.1875f;// 기존 0.25f

    const FVector Tip = Start + Forward * ArrowLength;
    const FVector HeadCenter = Tip - Forward * HeadLength;

    const FVector HeadPoints[4] =
    {
        HeadCenter + Right * HeadRadius,
        HeadCenter + Up * HeadRadius,
        HeadCenter - Right * HeadRadius,
        HeadCenter - Up * HeadRadius
    };

    const FVector4& Color = GetLightColor();

    // 몸통
    LineBatcher->AddLine(Start, Tip, Color);

    // 사각뿔 모양의 화살촉
    for (int32 i = 0; i < 4; ++i)
    {
        LineBatcher->AddLine(Tip, HeadPoints[i], Color);
        LineBatcher->AddLine(HeadPoints[i],HeadPoints[(i + 1) % 4], Color);
    }
}