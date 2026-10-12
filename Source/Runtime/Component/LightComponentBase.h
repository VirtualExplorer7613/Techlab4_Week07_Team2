#pragma once

#include "SceneComponent.h"

class ULightComponentBase : public USceneComponent
{
	DECLARE_CLASS(ULightComponentBase, USceneComponent)
	REFLECT_START(ULightComponentBase)
		PROPERTY(Intensity)
		PROPERTY_TYPE(LightColor, Color)
		PROPERTY(bVisible)
	REFLECT_END()

public:
	ULightComponentBase() = default;
	virtual ~ULightComponentBase() override;

	virtual void InitializeComponent() override;
	void SetIntensity(float NewIntensity) { Intensity = NewIntensity; }
	float GetIntensity() const { return Intensity; }
	void SetLightColor(const FVector4& NewColor) { LightColor = NewColor; }
	FVector4 GetLightColor() const { return LightColor; }
	void SetVisible(bool bNewVisible) { bVisible = bNewVisible; }
	bool IsVisible() const { return bVisible; }

protected:
	float Intensity = 1.0f;
	FVector4 LightColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	bool bVisible = true;
};
