#include "EnginePCH.h"
#include "LightActor.h"

#include "Asset/AssetManager.h"

namespace
{
	// 아이콘 빌보드에 라이트 아이콘 머티리얼을 끼운다. 색은 빌보드가 같은 액터의 라이트에서 읽는다.
	void UseLightIcon(UBillboardComponent* Billboard)
	{
		if (UMaterial* IconMaterial = UAssetManager::GetAssetByPath<UMaterial>("LightIconMaterial"))
			Billboard->SetMaterial(0, IconMaterial);
	}
}

AAmbientLightActor::AAmbientLightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	BillboardComponent->SetupAttachment(GetRootComponent());
	UseLightIcon(BillboardComponent);

	LightComponent = CreateDefaultSubobject<UAmbientLightComponent>("UAmbientLightComponent");
	LightComponent->SetupAttachment(GetRootComponent());
	LightComponent->SetIntensity(0.1f);   // 환경광은 약하게 깐다
}

ADirectionalLightActor::ADirectionalLightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	BillboardComponent->SetupAttachment(GetRootComponent());
	UseLightIcon(BillboardComponent);

	LightComponent = CreateDefaultSubobject<UDirectionalLightComponent>("UDirectionalLightComponent");
	LightComponent->SetupAttachment(GetRootComponent());
}

APointLightActor::APointLightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	BillboardComponent->SetupAttachment(GetRootComponent());
	UseLightIcon(BillboardComponent);

	LightComponent = CreateDefaultSubobject<UPointLightComponent>("UPointLightComponent");
	LightComponent->SetupAttachment(GetRootComponent());
}

ASpotLightActor::ASpotLightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	BillboardComponent->SetupAttachment(GetRootComponent());
	UseLightIcon(BillboardComponent);

	LightComponent = CreateDefaultSubobject<USpotLightComponent>("USpotLightComponent");
	LightComponent->SetupAttachment(GetRootComponent());
}
