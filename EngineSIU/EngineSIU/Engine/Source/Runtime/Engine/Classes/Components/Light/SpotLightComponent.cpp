#include "SpotLightComponent.h"
USpotLightComponent::USpotLightComponent()
{
    Type = ELightType::SPOT_LIGHT;
}

USpotLightComponent::~USpotLightComponent()
{
}

FVector USpotLightComponent::GetDirection()
{
    return Direction;
}

void USpotLightComponent::SetDirection(const FVector& dir)
{
    Direction = dir;
}
