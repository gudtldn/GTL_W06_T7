#pragma once

#include "Components/SceneComponent.h"

#include "Math/Color.h"

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();

public:
    float GetIntensity();
    FColor GetLightColor();
    bool IsVisible() const;
    
protected:
    float Intensity;
    FColor LightColor;
    bool bVisible;
    
};
