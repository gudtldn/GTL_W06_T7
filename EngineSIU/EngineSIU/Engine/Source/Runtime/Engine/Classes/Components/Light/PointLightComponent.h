#pragma once
#include "LightComponent.h"

class UPointLightComponent :public ULightComponent
{

    DECLARE_CLASS(UPointLightComponent, ULightComponent)
public:
    UPointLightComponent();
    virtual ~UPointLightComponent() override;

private:
    float AttenuationRadius;
    float LightFalloffExponent;
    
};


