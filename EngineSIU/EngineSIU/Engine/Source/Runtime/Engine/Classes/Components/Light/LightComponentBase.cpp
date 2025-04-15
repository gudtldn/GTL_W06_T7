#include "LightComponentBase.h"

ULightComponentBase::ULightComponentBase()
{
}

float ULightComponentBase::GetIntensity()
{
    return Intensity;
}

FColor ULightComponentBase::GetLightColor()
{
    return LightColor;
}

bool ULightComponentBase::IsVisible() const
{
    return bVisible;
}
