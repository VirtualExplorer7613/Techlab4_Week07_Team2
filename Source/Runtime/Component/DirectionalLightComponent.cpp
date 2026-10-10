#include "EnginePCH.h"
#include "DirectionalLightComponent.h"

void UDirectionalLightComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Direction = FVector(0.5f, 0.5f, -1.0f);
	LightColor = FVector(0.5f, 0.5f, 0.5f);
}
