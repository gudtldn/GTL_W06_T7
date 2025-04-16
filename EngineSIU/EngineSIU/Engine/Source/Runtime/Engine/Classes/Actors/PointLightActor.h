#pragma once
#include "LightActor.h"
#include "Components/Light/PointLightComponent.h"

class APointLight : public ALight
{
    DECLARE_CLASS(APointLight, ALight)
public:
    APointLight();

protected:

    UPROPERTY
    (UPointLightComponent*, PointLightComponent, = nullptr);

    UPROPERTY
    (UBillboardComponent*, BillboardComponent, = nullptr);
};

