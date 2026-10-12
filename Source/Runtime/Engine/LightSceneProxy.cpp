#include "EnginePCH.h"
#include "LightSceneProxy.h"
#include "Component/LightComponent.h"
#include "Component/SpotLightComponent.h"

FLightSceneProxy::~FLightSceneProxy()
{

}

void FLightSceneProxy::UpdateTransform()
{
	LightToWorld = Component->GetWorldMatrix();
	WorldToLight = LightToWorld.Inverse();
}

void FLightSceneProxy::UpdateParameters()
{
	// Scene Component 의 Property
	Position = Component->GetWorldLocation();
	Direction = Component->GetTransform().GetForward();
	
	// Light Component 의 Property
	LinearColor = Component->LightColor;
	Intensity = Component->Intensity;
	bVisible = Component->bVisible;
	
	Radius = Component->Radius;

	if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(Component))
	{
		FallOff = SpotLight->GetFalloff();
		InnerConeCos = cos(SpotLight->GetInnerConeAngle() * PI / 180.0f);
		OuterConeCos = cos(SpotLight->GetOuterConeAngle() * PI / 180.0f);
	}
	
	LightType = Component->GetLightType();
}

