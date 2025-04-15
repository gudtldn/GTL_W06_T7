#pragma once
#include "LightComponent.h"

class USpotLightComponent :public ULightComponent
{

    DECLARE_CLASS(USpotLightComponent, ULightComponent)
public:
    USpotLightComponent();
    ~USpotLightComponent();
    FVector GetDirection();
    void SetDirection(const FVector& dir);

private:
    float InnerConeAngle;
    float OuterConeAngle;
};

