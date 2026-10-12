#pragma once

#include "SceneComponent.h"
#include "Math/Vector4.h"

enum class ELightType
{
	Point = 0, Spot, Directional, Ambient
};


class ULightComponentBase : public USceneComponent
{
	DECLARE_CLASS(ULightComponentBase, USceneComponent)
	REFLECT_START(ULightComponentBase)
		PROPERTY(Intensity)
		PROPERTY(Radius)
		PROPERTY(bVisible)
		PROPERTY_TYPE(LightColor, Color)
		REFLECT_END()
public:
	float Intensity = 1.0f;
	float Radius = 5.0f;

	FVector4 LightColor = FVector4(1.0f, 1.0f, 0.6f, 1.0f);
	bool bVisible;

private:

}; 