#include "LightComponent.h"
#include "Components/BillboardComponent.h"
#include "UObject/Casts.h"

ULightComponent::ULightComponent()
{
    // FString name = "SpotLight";
    // SetName(name);
    InitializeLight();
}

ULightComponent::~ULightComponent()
{
  
}

UObject* ULightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->Light = Light;

    return NewComponent;
}

void ULightComponent::SetDiffuseColor(FLinearColor NewColor)
{
    Light.DiffuseColor = FVector(NewColor.R, NewColor.G, NewColor.B);
}

void ULightComponent::SetSpecularColor(FLinearColor NewColor)
{
   Light.SpecularColor = FVector(NewColor.R, NewColor.G, NewColor.B);
}

void ULightComponent::SetAttenuation(float Attenuation)
{
    Light.Attenuation = Attenuation;
}

void ULightComponent::SetAttenuationRadius(float AttenuationRadius)
{
    Light.AttRadius = AttenuationRadius;
}

void ULightComponent::SetIntensity(float Intensity)
{
    Light.Intensity = Intensity;
}

void ULightComponent::SetFalloff(float fallOff)
{
    Light.Falloff = fallOff;
}

FLinearColor ULightComponent::GetDiffuseColor()
{
    return FLinearColor(Light.DiffuseColor.X, Light.DiffuseColor.Y, Light.DiffuseColor.Z, 1);
}

FLinearColor ULightComponent::GetSpecularColor()
{
    return FLinearColor(Light.SpecularColor.X, Light.SpecularColor.Y, Light.SpecularColor.Z, 1);
}

float ULightComponent::GetAttenuation()
{
    return Light.Attenuation;
}

float ULightComponent::GetAttenuationRadius()
{
    return Light.AttRadius;
}

float ULightComponent::GetFalloff()
{
    return Light.Falloff;
}

void ULightComponent::InitializeLight()
{  
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    
    Light = FLight();
    Light.Enabled = 1;
}

void ULightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int ULightComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res = AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

