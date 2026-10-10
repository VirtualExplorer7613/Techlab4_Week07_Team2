#pragma once

#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponent
{
	DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)
	REFLECT_START(ClassName)
		PROPERTY_TYPE(Direction, Vector)
	REFLECT_END()

public:
	UDirectionalLightComponent() = default;
	virtual void InitializeComponent() override;

	FVector GetDirection() const { return Direction; }
	void SetDirection(const FVector& InDirection) { Direction = InDirection; }

protected:
	FVector Direction = { 0.5f, 0.5f, -1.0f };
};
