#include "SpotLightActor.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/BillboardComponent.h"

ASpotLight::ASpotLight()
{
    SpotLightComponent = AddComponent<USpotLightComponent>();

    BillboardComponent = AddComponent<UBillboardComponent>();

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/SpotLight_64x.png");

    RootComponent = BillboardComponent;

    SpotLightComponent->AttachToComponent(RootComponent);
}
