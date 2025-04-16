#pragma once
#include "LightActor.h"
#include "Components/Light/SpotLightComponent.h"

class ASpotLight : public ALight
{
    DECLARE_CLASS(ASpotLight, ALight)
public:
    ASpotLight();

protected:

    UPROPERTY
    (USpotLightComponent*, SpotLightComponent, = nullptr);

    UPROPERTY
    (UBillboardComponent*, BillboardComponent, = nullptr);
};


