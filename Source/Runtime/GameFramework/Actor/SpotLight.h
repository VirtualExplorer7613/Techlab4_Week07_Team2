#pragma once

#include "Info.h"
#include "Component/SpotLightComponent.h"

// 에디터에서 아이콘(빌보드)으로 보이고, 선택하면 스포트라이트 원뿔이 라인으로 그려진다.
class ASpotLight : public AInfo
{
	DECLARE_CLASS(ASpotLight, AInfo)

	REFLECT_START(ClassName)
	REFLECT_END()

public:
	ASpotLight();
	virtual ~ASpotLight() override = default;

	USpotLightComponent* GetSpotLightComponent() const { return SpotLightComponent; }

private:
	USpotLightComponent* SpotLightComponent = nullptr;
};
