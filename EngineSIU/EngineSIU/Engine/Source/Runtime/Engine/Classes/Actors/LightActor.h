#pragma once
#include "GameFramework/Actor.h"

class UPointLightComponent;
class USpotLightComponent;
class UBillboardComponent;

class ALight :public AActor
{
    DECLARE_CLASS(ALight, AActor)
public:
    ALight();
    virtual ~ALight();
protected:

};
