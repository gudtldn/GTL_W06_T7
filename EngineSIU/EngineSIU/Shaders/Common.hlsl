#ifndef MATERIALCOMMON_HLSL
#define MATERIALCOMMON_HLSL

struct FMaterial
{
    float3 DiffuseColor;
    float  TransparencyScalar;
    
    float3 AmbientColor;
    float  DensityScalar;
    
    float3 SpecularColor;
    float  SpecularScalar;
    
    float3 EmissiveColor;
    float  MaterialPad0;
};

#endif
