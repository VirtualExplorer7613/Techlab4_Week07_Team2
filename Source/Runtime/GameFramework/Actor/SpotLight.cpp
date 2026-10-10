#include "EnginePCH.h"
#include "SpotLight.h"
#include "Asset/AssetManager.h"
#include "Render/Material.h"

ASpotLight::ASpotLight()
{
	SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>("USpotLightComponent");
	SpotLightComponent->SetupAttachment(GetRootComponent());

	UMaterial* Material = UAssetManager::GetAssetByPath<UMaterial>("SpotLightIconMaterial");
	assert(Material);
	GetBillboardComponent()->SetMaterial(0, Material);
}
