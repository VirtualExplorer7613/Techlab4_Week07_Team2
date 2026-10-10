#include "EnginePCH.h"
#include "AmbientLightComponent.h"

void UAmbientLightComponent::InitializeComponent()
{
	Super::InitializeComponent();
	LightColor = FVector(0.5f, 0.5f, 0.5f);
	Intensity = 0.5f;
}
