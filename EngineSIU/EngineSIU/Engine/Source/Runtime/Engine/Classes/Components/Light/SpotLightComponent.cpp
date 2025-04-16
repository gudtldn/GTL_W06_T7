#include "SpotLightComponent.h"
USpotLightComponent::USpotLightComponent()
{
    Light.Type = ELightType::SPOT_LIGHT;
}

USpotLightComponent::~USpotLightComponent()
{
}

FVector USpotLightComponent::GetDirection()
{
    return Light.Direction;
}

void USpotLightComponent::SetDirection(const FVector& dir)
{
    Light.Direction = dir;
}

float USpotLightComponent::GetInnerConeAngle() const
{
    return InnerConeAngle;
}

float USpotLightComponent::GetOuterConeAngle() const
{
    return OuterConeAngle;
}

void USpotLightComponent::SetInnerConeAngle(float NewAngle)
{
    InnerConeAngle = NewAngle;
}

void USpotLightComponent::SetOuterConeAngle(float NewAngle)
{
    OuterConeAngle = NewAngle;
}

float USpotLightComponent::GetFalloff() const
{
    return Light.Falloff;
}

void USpotLightComponent::SetFalloff(float NewFalloff)
{
    Light.Falloff = NewFalloff;
}
