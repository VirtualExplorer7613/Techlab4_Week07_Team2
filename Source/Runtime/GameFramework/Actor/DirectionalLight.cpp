
#include "EnginePCH.h"
#include "DirectionalLight.h"
#include "Asset/AssetManager.h"
#include "Render/Material.h"

ADirectionalLight::ADirectionalLight()
{
	DirectionalLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>("UDirectionalLightComponent");
	DirectionalLightComponent->SetupAttachment(GetRootComponent());

	UMaterial* Material = UAssetManager::GetAssetByPath<UMaterial>("DirectionalLightIconMaterial");
	assert(Material);
	GetBillboardComponent()->SetMaterial(0, Material);
}
