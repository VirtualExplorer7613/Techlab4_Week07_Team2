#pragma once

#include "LightComponent.h"

class UPointLightComponent : public ULightComponent
{
    DECLARE_CLASS(UPointLightComponent, ULightComponent)

    REFLECT_START(UPointLightComponent)
        PROPERTY(AttenuationRadius)
        PROPERTY(RadiusFalloff)
    REFLECT_END()

public:
    ~UPointLightComponent() override;
    void InitializeComponent() override;

    float GetAttenuationRadius() const
    {
        return AttenuationRadius > 0.0f ? AttenuationRadius : 0.0f;
    }

    void SetAttenuationRadius(float InRadius)
    {
        AttenuationRadius = InRadius > 0.0f ? InRadius : 0.0f;
    }

    float GetRadiusFalloff() const
    {
        return RadiusFalloff > 0.001f ? RadiusFalloff : 0.001f;
    }

    void SetRadiusFalloff(float InFalloff)
    {
        RadiusFalloff = InFalloff > 0.001f ? InFalloff : 0.001f;
    }

    void PostEditChangeProperty(const FProperty& Property) override
    {
        Super::PostEditChangeProperty(Property);

        SetAttenuationRadius(AttenuationRadius);
        SetRadiusFalloff(RadiusFalloff);
    }

    void DrawDebug(FLineBatcher* LineBatcher) const override;

private:
    float AttenuationRadius = 5.0f;
    float RadiusFalloff = 2.0f;
};