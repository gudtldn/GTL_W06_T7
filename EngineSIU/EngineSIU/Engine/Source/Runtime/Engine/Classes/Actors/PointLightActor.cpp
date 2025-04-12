#include "PointLightActor.h"
#include "Components/PointLightComponent.h"
#include "Components/BillboardComponent.h"

APointLight::APointLight()
{
    PointLightComponent = AddComponent<UPointLightComponent>();

    BillboardComponent = AddComponent<UBillboardComponent>();

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/PointLight_64x.png");

    RootComponent = BillboardComponent;

    PointLightComponent->AttachToComponent(RootComponent);
}
