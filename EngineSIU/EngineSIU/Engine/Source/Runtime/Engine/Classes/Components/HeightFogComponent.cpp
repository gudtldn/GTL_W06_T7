#include "HeightFogComponent.h"
#include <UObject/Casts.h>

UHeightFogComponent::UHeightFogComponent(float Density, float HeightFalloff, float StartDist, float CutoffDist, float MaxOpacity)
    :FogDensity(Density), FogHeightFalloff(HeightFalloff), StartDistance(StartDist), FogCutoffDistance(CutoffDist), FogMaxOpacity(MaxOpacity)
{
    FogInscatteringColor = FLinearColor::White;
}

void UHeightFogComponent::SetFogDensity(float value)
{
    FogDensity = value;
}

void UHeightFogComponent::SetFogHeightFalloff(float value)
{
    FogHeightFalloff = value; 
}

void UHeightFogComponent::SetStartDistance(float value)
{
    StartDistance = value;
}

void UHeightFogComponent::SetFogCutoffDistance(float value)
{
    FogCutoffDistance = value;
}

void UHeightFogComponent::SetFogMaxOpacity(float value)
{
    FogMaxOpacity = value;
}

void UHeightFogComponent::SetFogColor(FLinearColor color)
{
    FogInscatteringColor = color;
}

UObject* UHeightFogComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->FogDensity = FogDensity;
    NewComponent->FogHeightFalloff = FogHeightFalloff;
    NewComponent->StartDistance = StartDistance;
    NewComponent->FogCutoffDistance = FogCutoffDistance;
    NewComponent->FogMaxOpacity = FogMaxOpacity;
    NewComponent->FogInscatteringColor = FogInscatteringColor;

    return NewComponent;
}

void UHeightFogComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("FogDensity"), FString::Printf(TEXT("%f"), FogDensity));
    OutProperties.Add(TEXT("FogHeightFalloff"), FString::Printf(TEXT("%f"), FogHeightFalloff));
    OutProperties.Add(TEXT("StartDistance"), FString::Printf(TEXT("%f"), StartDistance));
    OutProperties.Add(TEXT("FogCutoffDistance"), FString::Printf(TEXT("%f"), FogCutoffDistance));
    OutProperties.Add(TEXT("FogMaxOpacity"), FString::Printf(TEXT("%f"), FogMaxOpacity));
    //FVector4 Color = FVector4(FogInscatteringColor.R, FogInscatteringColor.G, FogInscatteringColor.B, FogInscatteringColor.A);
    
    OutProperties.Add(TEXT("FogInscatteringColor"), FString::Printf(TEXT("%s"), *FogInscatteringColor.ToString()));
}

void UHeightFogComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("FogDensity"));
    if (TempStr)
    {
        FogDensity = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FogHeightFalloff"));
    if (TempStr)
    {
        FogHeightFalloff = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("StartDistance"));
    if (TempStr)
    {
        StartDistance = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FogCutoffDistance"));
    if (TempStr)
    {
        FogCutoffDistance = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FogMaxOpacity"));
    if (TempStr)
    {
        FogMaxOpacity = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FogInscatteringColor"));
    if (TempStr)
    {
        // FVector4 Color;
        // Color.InitFromString(*TempStr);
        FogInscatteringColor = FLinearColor(*TempStr);
    }
}
