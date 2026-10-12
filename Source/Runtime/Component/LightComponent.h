#pragma once

#include "LightComponentBase.h"

class FLineBatcher;
class FLightSceneProxy;

class ULightComponent : public ULightComponentBase
{
	DECLARE_CLASS(ULightComponent, USceneComponent)
	REFLECT_START(ULightComponent)
	REFLECT_END()
public:
	// 상속받은 Light 들 override 해줘야함.
	virtual ELightType GetLightType() const {
		return ELightType::Ambient;
	}

	virtual void DrawDebug(FLineBatcher* LineBatcher) const {}
	
private:
	friend class FScene;
	FLightSceneProxy* SceneLightProxy = nullptr;
};