#pragma once
#include "LightComponent.h"

class UPointLightComponent : public ULightComponent
{
	DECLARE_CLASS(UPointLightComponent, ULightComponent)

	REFLECT_START(ClassName)
		PROPERTY(RadiusFalloff)
		REFLECT_END()

public:
	UPointLightComponent() = default;
	virtual ~UPointLightComponent() override;
	virtual void InitializeComponent() override;

	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;

	float GetRadiusFalloff() const { return RadiusFalloff; }

	virtual ELightType GetLightType() const override{
		return ELightType::Point;
	}


private:
	float RadiusFalloff = 3.0f;
};
