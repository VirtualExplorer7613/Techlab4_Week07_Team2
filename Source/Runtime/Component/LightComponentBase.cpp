#include "EnginePCH.h"
#include "LightComponentBase.h"
#include "Engine/World.h"

ULightComponentBase::~ULightComponentBase()
{
	if (AActor* Owner = GetOwner())
	{
		if (UWorld* World = Owner->GetWorld())
		{
			World->GetScene().UnregisterLight(this);
		}
	}
}

void ULightComponentBase::InitializeComponent()
{
    Super::InitializeComponent();

    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().RegisterLight(this);
        }
    }
}


