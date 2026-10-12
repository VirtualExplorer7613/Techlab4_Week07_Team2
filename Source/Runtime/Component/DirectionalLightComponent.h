#pragma once

#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponent
{
	DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)
public:
	UDirectionalLightComponent() = default;
	virtual ~UDirectionalLightComponent() override = default;
};