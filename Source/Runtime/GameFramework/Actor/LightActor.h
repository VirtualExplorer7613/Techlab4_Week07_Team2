#pragma once

#include "GameFramework/Actor.h"
#include "Component/BillboardComponent.h"
#include "Component/AmbientLightComponent.h"
#include "Component/DirectionalLightComponent.h"
#include "Component/PointLightComponent.h"
#include "Component/SpotLightComponent.h"

// 라이트 액터들은 에디터에서 라이트 색으로 칠해진 아이콘(빌보드)으로 보이고,
// 선택하면 라이트 종류에 맞는 와이어프레임(화살표·반경·원뿔)이 그려진다.
// 빌보드는 클릭해서 고를 수 있는 프리미티브라 함께 붙인다.

class AAmbientLightActor : public AActor
{
	DECLARE_CLASS(AAmbientLightActor, AActor)

public:
	AAmbientLightActor();
	virtual ~AAmbientLightActor() override = default;

	UAmbientLightComponent* GetLightComponent() const { return LightComponent; }

private:
	UBillboardComponent* BillboardComponent = nullptr;
	UAmbientLightComponent* LightComponent = nullptr;
};

class ADirectionalLightActor : public AActor
{
	DECLARE_CLASS(ADirectionalLightActor, AActor)

public:
	ADirectionalLightActor();
	virtual ~ADirectionalLightActor() override = default;

	UDirectionalLightComponent* GetLightComponent() const { return LightComponent; }

private:
	UBillboardComponent* BillboardComponent = nullptr;
	UDirectionalLightComponent* LightComponent = nullptr;
};

class APointLightActor : public AActor
{
	DECLARE_CLASS(APointLightActor, AActor)

public:
	APointLightActor();
	virtual ~APointLightActor() override = default;

	UPointLightComponent* GetLightComponent() const { return LightComponent; }

private:
	UBillboardComponent* BillboardComponent = nullptr;
	UPointLightComponent* LightComponent = nullptr;
};

class ASpotLightActor : public AActor
{
	DECLARE_CLASS(ASpotLightActor, AActor)

public:
	ASpotLightActor();
	virtual ~ASpotLightActor() override = default;

	USpotLightComponent* GetLightComponent() const { return LightComponent; }

private:
	UBillboardComponent* BillboardComponent = nullptr;
	USpotLightComponent* LightComponent = nullptr;
};
