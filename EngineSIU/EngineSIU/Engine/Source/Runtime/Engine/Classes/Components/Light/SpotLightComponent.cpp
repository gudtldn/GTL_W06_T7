#include "SpotLightComponent.h"
USpotLightComponent::USpotLightComponent()
{
    DiffuseColor = FVector(1.0f, 1.0f, 1.0f);       // 기본 디퓨즈 흰색
    SpecularColor = FVector(1.0f, 1.0f, 1.0f);      // 기본 스펙큘러 흰색
    Attenuation = 1.0f;                             // 기본 감쇠 값
    AttRadius = 1000.0f;                            // 감쇠 반경
    Intensity = 1.0f;                               // 광원 강도
    Falloff = 1.0f;                                 // 스팟 라이트 falloff 계수
    Direction = FVector(0.0f, -1.0f, 0.0f);          // 기본 광원
    InnerConeAngle = 15;
    OuterConeAngle = 30;
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
    return Falloff;
}

void USpotLightComponent::SetFalloff(float NewFalloff)
{
    Falloff = NewFalloff;
}
