#pragma once

#include "LightComponent.h"

class UPointLightComponent : public ULightComponent
{
	DECLARE_CLASS(UPointLightComponent, ULightComponent)
	REFLECT_START(UPointLightComponent)
		PROPERTY(AttenuationRadius)
		PROPERTY(LightFalloffExponent)
	REFLECT_END()

public:
	UPointLightComponent() = default;
	virtual ~UPointLightComponent() override = default;

	// AttenuationRadius 크기의 구를 세 평면의 원으로 그린다.
	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	float GetAttenuationRadius() const { return AttenuationRadius; }
	void SetAttenuationRadius(float NewRadius) { AttenuationRadius = NewRadius; }
	float GetLightFalloffExponent() const { return LightFalloffExponent; }
	void SetLightFalloffExponent(float NewExponent) { LightFalloffExponent = NewExponent; }

protected:
	float AttenuationRadius = 6.5f;
	float LightFalloffExponent = 3.0f;
};