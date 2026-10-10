
#include "EnginePCH.h"
#include "AmbientLight.h"
#include "Asset/AssetManager.h"
#include "Render/Material.h"

AAmbientLight::AAmbientLight()
{
	AmbientLightComponent = CreateDefaultSubobject<UAmbientLightComponent>("UAmbientLightComponent");
	AmbientLightComponent->SetupAttachment(GetRootComponent());

	UMaterial* Material = UAssetManager::GetAssetByPath<UMaterial>("AmbientLightIconMaterial");
	assert(Material);
	GetBillboardComponent()->SetMaterial(0, Material);
}
