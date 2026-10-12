#include "EnginePCH.h"
#include "DirectionalLightActor.h"

#include "Component/BillboardComponent.h"
#include "Component/DirectionalLightComponent.h"

ADirectionalLightActor::ADirectionalLightActor()
{
    UDirectionalLightComponent* Light = CreateDefaultSubobject<UDirectionalLightComponent>("UDirectionalLightComponent");

    Light->SetupAttachment(GetRootComponent());

    UBillboardComponent* Billboard = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");

    Billboard->SetupAttachment(Light);
    Billboard->SetHiddenInDetails(true);
}