#pragma once

#include "LightComponent.h"

class UAmbientLightComponent : public ULightComponent
{
    DECLARE_CLASS(UAmbientLightComponent, ULightComponent)

    REFLECT_START(UAmbientLightComponent)
        REFLECT_END()

public:
    UAmbientLightComponent()
    {
        SetIntensity(0.2f);
    }

    ~UAmbientLightComponent() override;

    void InitializeComponent() override;
};