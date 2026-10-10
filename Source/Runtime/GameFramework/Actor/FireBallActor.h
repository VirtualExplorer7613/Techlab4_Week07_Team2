#pragma once
#include "GameFramework/Actor.h"
#include "Component/PointLightComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/RotatingMovementComponent.h"
#include "Component/ProjectileMovementComponent.h"

class AFireBallActor : public AActor
{
	DECLARE_CLASS(AFireBallActor, AActor)

public:
	AFireBallActor();
	UPointLightComponent* GetFireBallComponent() const { return PointLightComponent; }

private:
	UPointLightComponent* PointLightComponent;
	UStaticMeshComponent* StaticMeshComponent;

	URotatingMovementComponent* RotatingMovementComponent;
	UProjectileMovementComponent* ProjectileMovementComponent;
};