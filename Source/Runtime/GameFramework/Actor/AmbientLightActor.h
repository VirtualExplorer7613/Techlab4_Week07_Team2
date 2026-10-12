#pragma once

#include "GameFramework/Actor.h"
#include "Component/BillboardComponent.h"
#include "Component/AmbientLightComponent.h"

class AAmbientLightActor : public AActor
{
    DECLARE_CLASS(AAmbientLightActor, AActor)

    REFLECT_START(AAmbientLightActor)
    REFLECT_END()

public:
    AAmbientLightActor();
    ~AAmbientLightActor() override = default;

	UBillboardComponent* GetBillboardComponent() const { return BillboardComponent; }
	UAmbientLightComponent* GetAmbientLightComponent() const { return AmbientLightComponent; }

private:
	// 클릭해서 고를 수 있어야 하므로 프리미티브인 빌보드를 루트로 둔다
	UBillboardComponent* BillboardComponent = nullptr;

	UAmbientLightComponent* AmbientLightComponent = nullptr;
};