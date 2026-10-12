#include "EnginePCH.h"
#include "Component/AmbientLightComponent.h"

#include "Engine/World.h"
#include "Engine/Scene.h"

UAmbientLightComponent::~UAmbientLightComponent()
{
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().UnregisterAmbientLight(this);
        }
    }
}

void UAmbientLightComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().RegisterAmbientLight(this);
        }
    }
}