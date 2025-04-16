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

    // ConeAngle에 대한 Getter Setter 함수 만들어 주기
    // ConeAngle Getters
    float GetInnerConeAngle() const;
    float GetOuterConeAngle() const;

    // ConeAngle Setters
    void SetInnerConeAngle(float NewAngle);
    void SetOuterConeAngle(float NewAngle);

    // Get/Set for Falloff
    float GetFalloff() const;
    void SetFalloff(float NewFalloff);

private:
    float InnerConeAngle;
    float OuterConeAngle;
};

