#include "EnginePCH.h"
#include "PointLight.h"
#include "Asset/AssetManager.h"
#include "Render/Texture2D.h"
#include "Render/Material.h"

APointLight::APointLight()
{
	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>("UPointLightComponent");
	PointLightComponent->SetupAttachment(GetRootComponent());

	UMaterial* Material = UAssetManager::GetAssetByPath<UMaterial>("PointLightIconMaterial");
	GetBillboardComponent()->SetMaterial(0, Material);

	//RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>("URotatingMovementComponent");	
	//RotatingMovementComponent->RotationRate = FRotator(0.0f, 100.0f, 100.0f);
	//RotatingMovementComponent->PivotTranslation = FVector(0.0f, 0.0f, 0.0f);

	//ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("UProjectileMovementComponent");
	//ProjectileMovementComponent->InintialSpeed = 1.0f;
	//ProjectileMovementComponent->MaxSpeed = 5.0f;
	//ProjectileMovementComponent->Velocity = FVector(5.0f, 0.0f, 0.0f);
	//ProjectileMovementComponent->bShouldBounce = false;
	//ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;
	//ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}
