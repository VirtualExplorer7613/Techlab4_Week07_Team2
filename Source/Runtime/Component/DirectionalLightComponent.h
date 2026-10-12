#pragma once

#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponent
{
    DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)

    REFLECT_START(UDirectionalLightComponent)
    REFLECT_END()

public:
    ~UDirectionalLightComponent() override;
    void InitializeComponent() override;
    // 빛이 진행하는 월드 방향.
    // 나중에 표면 -> 광원 방향 L로 사용할 때는 부호를 뒤집는다.
    FVector GetLightDirection() const
    {
        FTransform WorldTransform;
        WorldTransform.Rotation = GetWorldRotation();

        return WorldTransform.GetForward().GetSafeNormal();
    }

    void DrawDebug(FLineBatcher* LineBatcher) const override;
};