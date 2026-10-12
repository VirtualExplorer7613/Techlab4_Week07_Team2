#pragma once

#include "LightComponentBase.h"


class FLightSceneProxy;

class ULightComponent : public ULightComponentBase
{
	DECLARE_CLASS(ULightComponent, USceneComponent)
	REFLECT_START(ULightComponent)
	REFLECT_END()
public:
	// 상속받은 Light 들 override 해줘야힘.
	virtual ELightType GetLightType() const {
		return ELightType::Ambient;
	}
	
private:
	friend class FScene;
	FLightSceneProxy* SceneLightProxy = nullptr;
};