
#include "EnginePCH.h"
#include "DirectionalLight.h"
#include "Asset/AssetManager.h"
#include "Render/Material.h"
#include "Asset/AssetManager.h"

ADirectionalLight::ADirectionalLight()
{
	DirectionalLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>("UDirectionalLightComponent");
	DirectionalLightComponent->SetupAttachment(GetRootComponent());

	//TODO : 에디터에서만 보이게 해야 한다.
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("ArrowComponent");
	StaticMeshComponent->SetupAttachment(DirectionalLightComponent);

	StaticMeshComponent->SetStaticMesh(UAssetManager::GetAssetByPath<UStaticMesh>("Arrow"));
	StaticMeshComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	StaticMeshComponent->SetWorldScale3D(FVector(0.5f));


	UMaterial* Material = UAssetManager::GetAssetByPath<UMaterial>("DirectionalLightIconMaterial");
	assert(Material);
	GetBillboardComponent()->SetMaterial(0, Material);
}
