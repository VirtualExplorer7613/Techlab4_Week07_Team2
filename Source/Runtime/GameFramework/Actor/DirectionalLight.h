#pragma once

#include "Info.h"
#include "Component/DirectionalLightComponent.h"

class ADirectionalLight : public AInfo
{
	DECLARE_CLASS(ADirectionalLight, AInfo)

	REFLECT_START(ClassName)
		REFLECT_END()

public:
	ADirectionalLight();
	virtual ~ADirectionalLight() override = default;

	UDirectionalLightComponent* GetDirctionalLightComponent() const { return DirectionalLightComponent; }

private:
	UDirectionalLightComponent* DirectionalLightComponent = nullptr;
};
