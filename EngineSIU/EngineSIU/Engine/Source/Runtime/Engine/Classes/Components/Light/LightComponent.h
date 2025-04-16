#pragma once
#include "LightComponentBase.h"
#include "Define.h"
class UBillboardComponent;

class ULightComponent : public ULightComponentBase
{
    DECLARE_CLASS(ULightComponent, ULightComponentBase)

public:
    ULightComponent();
    virtual ~ULightComponent() override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    void InitializeLight();
    
    void SetDiffuseColor(FLinearColor NewColor);
    void SetSpecularColor(FLinearColor NewColor);
    void SetAttenuation(float Attenuation);
    void SetAttenuationRadius(float AttenuationRadius);
    void SetIntensity(float Intensity);
    void SetFalloff(float fallOff);

    FLinearColor GetDiffuseColor();
    FLinearColor GetSpecularColor();
    float GetAttenuation();
    float GetAttenuationRadius();
    float GetFalloff();
protected:
    FBoundingBox AABB;
    
    FVector DiffuseColor;
    FVector SpecularColor;
    float Attenuation;
    float AttRadius;
    float Intensity;
    float Falloff;
    // ELightType Type;
    FVector Direction;

public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    
    float GetIntensity() const {return Intensity;}
    
};
