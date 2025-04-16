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

    // NewComponent->Light = Light;
    NewComponent->DiffuseColor = DiffuseColor;
    NewComponent->SpecularColor = SpecularColor;
    NewComponent->Attenuation = Attenuation;
    NewComponent->AttRadius = AttRadius;
    NewComponent->Intensity = Intensity;
    NewComponent->Falloff = Falloff;
    //NewComponent-> Type = Type;
    NewComponent-> Direction = Direction;
    
    return NewComponent;
}

void ULightComponent::SetDiffuseColor(FLinearColor NewColor)
{
    DiffuseColor = FVector(NewColor.R, NewColor.G, NewColor.B);
}

void ULightComponent::SetSpecularColor(FLinearColor NewColor)
{
   SpecularColor = FVector(NewColor.R, NewColor.G, NewColor.B);
}

void ULightComponent::SetAttenuation(float Attenuation)
{
    this->Attenuation = Attenuation;
}

void ULightComponent::SetAttenuationRadius(float AttenuationRadius)
{
    AttRadius = AttenuationRadius;
}

void ULightComponent::SetIntensity(float Intensity)
{
    this->Intensity = Intensity;
}

void ULightComponent::SetFalloff(float fallOff)
{
    Falloff = fallOff;
}

FLinearColor ULightComponent::GetDiffuseColor()
{
    return FLinearColor(DiffuseColor.X, DiffuseColor.Y, DiffuseColor.Z, 1);
}

FLinearColor ULightComponent::GetSpecularColor()
{
    return FLinearColor(SpecularColor.X, SpecularColor.Y, SpecularColor.Z, 1);
}

float ULightComponent::GetAttenuation()
{
    return Attenuation;
}

float ULightComponent::GetAttenuationRadius()
{
    return AttRadius;
}

float ULightComponent::GetFalloff()
{
    return Falloff;
}

void ULightComponent::InitializeLight()
{  
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    
    // Light = FLighting();
    // Light.Enabled = 1;
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

