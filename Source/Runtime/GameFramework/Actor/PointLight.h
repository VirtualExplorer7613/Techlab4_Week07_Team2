#pragma once

#include "Info.h"
#include "Component/PointLightComponent.h"

class APointLight : public AInfo
{
	DECLARE_CLASS(APointLight, AInfo)
	REFLECT_START(ClassName)
		REFLECT_END()

public:
	APointLight();
	UPointLightComponent* GetPointLightComponent() const { return PointLightComponent; }

private:
	UPointLightComponent* PointLightComponent;
};