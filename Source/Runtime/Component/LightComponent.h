#pragma once

#include "SceneComponent.h"

class ULightComponent : public USceneComponent
{
	DECLARE_CLASS(ULightComponent, USceneComponent)
	REFLECT_START(ULightComponent)
		PROPERTY(Intensity)
		PROPERTY_TYPE(LightColor, Color)
		PROPERTY(bVisible)
	REFLECT_END()

public:
    ULightComponent() = default;
    virtual ~ULightComponent() override = default;

    float GetIntensity() const { return Intensity; }
    void SetIntensity(float InIntensity)
    {
        Intensity = InIntensity >= 0.0f ? InIntensity : 0.0f;
    }

    const FVector4& GetLightColor() const { return LightColor; }
    void SetLightColor(const FVector4& InColor)
    {
        LightColor = InColor;
    }

    bool IsVisible() const { return bVisible; }
    void SetVisibility(bool bInVisible)
    {
        bVisible = bInVisible;
    }

private:
    float Intensity = 1.0f;
    FVector4 LightColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    bool bVisible = true;
};