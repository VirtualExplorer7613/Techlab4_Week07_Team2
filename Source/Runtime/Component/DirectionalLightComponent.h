#pragma once

#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponent
{
	DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)
public:
	UDirectionalLightComponent() = default;
	virtual ~UDirectionalLightComponent() override = default;

	// 빛이 나아가는 방향(컴포넌트 Forward)으로 화살표를 그린다.
	virtual void DrawDebug(FLineBatcher* LineBatcher) const override;
};