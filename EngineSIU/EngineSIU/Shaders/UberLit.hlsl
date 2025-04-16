// UberLit.hlsl

#include <Common.hlsl>

#define MAX_LIGHTS                  16
#define NUM_POINT_LIGHT              4
#define NUM_SPOT_LIGHT               4

#define LIGHTING_MODEL_GOURAUD       1
#define LIGHTING_MODEL_LAMBERT       2
#define LIGHTING_MODEL_BLINNPHONG    3
#define LIGHTING_MODEL_UNLIT         4


struct AmbientLightInfo
{
    float3 Color;
    float  Intensity;
};

struct DirectionalLightInfo
{
    float3 Direction;
    float pad0;
    float3 Color;
    float  Intensity;
};

struct PointLightInfo
{
    float3 Position;
    float pad0;
    float3 DiffuseColor;
    float pad1;
    float3 SpecularColor;
    float pad2;
    float  Intensity;
    float  m_fAttRadius;
    float  m_fAttenuation; // 추가: 거리 기반 감쇠 계수
    float pad3;
};

struct SpotLightInfo
{
    float3 Position;
    float  pad0;
    float3 Direction;
    float  pad1;
    float3 DiffuseColor;
    float  pad2;
    float3 SpecularColor;
    float  pad3;
    float  Intensity;
    float  m_fAttRadius;
    float  m_fFalloff;      // 스팟라이트 감쇠 인자
    float  m_fAttenuation;  // 거리 기반 감쇠 계수
    float  InnerConeAngle;
    float  OuterConeAngle;
    float2 pad4;
};

cbuffer Lighting : register(b2)
{
    AmbientLightInfo AmbientLight;
    DirectionalLightInfo DirectionalLight;
    PointLightInfo PointLights[NUM_POINT_LIGHT];
    SpotLightInfo SpotLights[NUM_SPOT_LIGHT];
};

cbuffer MaterialConstants : register(b3)
{
    FMaterial Material;
}

//---------------------------------------------------------------------------
// 조명 함수들
//---------------------------------------------------------------------------
float4 CalculateSpotLight(SpotLightInfo light, float3 vPosition, float3 vNormal)
{
    float3 vToLightSource = light.Position - vPosition;
    float fDistanceToLight = length(vToLightSource);
    float3 vDirectionToLightSource = vToLightSource / fDistanceToLight;
    float3 normal = normalize(vNormal);

    float fSpotCosAngle =  dot(-vDirectionToLightSource, light.Direction); // length(gLights[nIndex].m_vDirection);
    
    
    float DistanceAttenuation = saturate(1.0f - fDistanceToLight / (light.m_fAttRadius));
    
    float CosInnerConeAngle = cos(radians(light.InnerConeAngle));
    float CosOuterConeAngle = cos(radians(light.OuterConeAngle));
    //float CosInnerConeAngle = cos(radians(10.0f / 2)); // 예시: 내부 원뿔 반각 30도의 코사인
    //float CosOuterConeAngle = cos(radians(60.0f / 2)); // 예시: 외부 원뿔 반각 60도의 코사인
    
    float SpotLightRatioBase = (fSpotCosAngle - CosOuterConeAngle) / (CosInnerConeAngle - CosOuterConeAngle));
    //float SpotLightRatioBase = (fSpotCosAngle - cosInnerConeAngle) / (cosInnerConeAngle - cosOuterConeAngle);
    float SaturatedSpotLightRatioBase = saturate(SpotLightRatioBase);
    
    float SpotFalloffExponent = light.m_fFalloff;
    float SpotAngleAttenuation = pow(SaturatedSpotLightRatioBase, SpotFalloffExponent);
    
    // Lambertian Diffuse 
    float3 DiffuseColorRGB = light.DiffuseColor;
    float LambertFactor = max(0.0f, dot(normal, vDirectionToLightSource));
    float3 DiffuseLightContribution = DiffuseColorRGB * LambertFactor;
    
    float CombinedAttenuation = DistanceAttenuation * SpotAngleAttenuation;

    // 최종 확산 색상 (감쇠 및 광원 강도 적용)
    float3 FinalDiffuseColor = DiffuseLightContribution * CombinedAttenuation * light.Intensity;
    
    float4 FinalColor = float4(FinalDiffuseColor, 1.0f);

    //return float4(DistanceAttenuation, DistanceAttenuation, DistanceAttenuation, 1.0f);
    return FinalColor;
    
}

float4 CalculatePointLight(PointLightInfo light, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = light.Position - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > light.m_fAttRadius)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    float fSpecularFactor = 0.0f;
    vToLight /= fDistance; // 정규화
    float fDiffuseFactor = saturate(dot(vNormal, vToLight));

    if (fDiffuseFactor > 0.0f)
    {
        float3 vView = normalize(CameraPosition - vPosition);
        float3 vHalf = normalize(vToLight + vView);
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 32);
    }

    float fAttenuationFactor = 1.0f / (1.0f + light.m_fAttenuation * fDistance * fDistance);
   
    float3 lit = (light.DiffuseColor.rgb * fDiffuseFactor * Material.DiffuseColor) +
                 (light.SpecularColor.rgb * fSpecularFactor * Material.SpecularColor);

    return float4(lit * fAttenuationFactor * light.Intensity, 1.0f);
}

float3 CalculateAmbientLight()
{
    return AmbientLight.Color * AmbientLight.Intensity * Material.AmbientColor.rgb;
}

float3 CalculateDirectionalLight(float3 normal)
{
    float3 N = normalize(normal);
    float diffuseFactor = saturate(dot(N, -DirectionalLight.Direction));
    // diffuseFactor = saturate(dot(N, -float3(1, -1, -1)));
    // return float4(1, 1, 1, 1) * 1 * diffuseFactor;
    
    return DirectionalLight.Color * DirectionalLight.Intensity * diffuseFactor;
}

float3 CalculateDirLight(DirectionalLightInfo light, float3 vPosition, float3 vNormal)
{
    float3 result = float3(0, 0, 0);
    float3 vToLight = normalize(-light.Direction);
    float NdotL = dot(vNormal, vToLight);

    if (NdotL <= 0.0f)
        return float4(0, 0, 0, 1);

    float3 safeDiffuse = (length(Material.DiffuseColor) < 0.001f) ? float3(1, 1, 1) : Material.DiffuseColor;
    float3 vView = normalize(CameraPosition - vPosition);
    float3 vHalf = normalize(vToLight + vView);
    float fSpecular = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 32);

#if LIGHTING_MODEL == LIGHTING_MODEL_GOURAUD
    return float4(0, 0, 0, 1);

#elif LIGHTING_MODEL == LIGHTING_MODEL_LAMBERT
    result = light.Color * NdotL * safeDiffuse;

#elif LIGHTING_MODEL == LIGHTING_MODEL_BLINNPHONG
    result = light.Color * NdotL * safeDiffuse + light.Color * fSpecular * Material.SpecularColor;
#endif

    return result * light.Intensity;
}

struct VertexInput
{
    float3 worldPos;
    float3 normalWS;
    float4 color;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 worldPos;
    float3 normal; 
    float2 texcoord;
    float4 color;
};

float4 Uber_VS(VertexInput input)
{
#if LIGHTING_MODEL == LIGHTING_MODEL_GOURAUD
    float3 lighting = float3(0.0f, 0.0f, 0.0f);

    // Ambient
    lighting += CalculateAmbientLight().rgb;

    // Point Lights
    for (int i=0; i<NUM_POINT_LIGHT; ++i)
    {
        lighting += CalculatePointLight(PointLights[i], input.worldPos, input.normalWS).rgb;
    }

    // Spot Lights
    for (int i=0; i<NUM_SPOT_LIGHT; ++i)
    {
        lighting += saturate(CalculateSpotLight(SpotLights[i], input.worldPos, input.normalWS).rgb);
    }
    
    // Directional Light
    lighting += CalculateDirectionalLight(input.normalWS);
    
    return float4(input.color.rgb * lighting, 1.0f);

#else
    return input.color;
#endif
}

#ifdef PIXEL_SHADER
float4 Uber_PS(PixelInput Input)
{
    float3 finalColor;

    // Base Color
    float3 textureColor = Textures.Sample(Sampler, Input.texcoord).rgb;
    float3 matDiffuse = Material.DiffuseColor.rgb;
    bool hasTexture = any(textureColor != float3(0, 0, 0));
    float3 baseColor = hasTexture ? textureColor : matDiffuse;
    
    float3 emissive = Material.EmissiveColor;
    float3 normal = normalize(Input.normal);
    float3 ambient = AmbientLight.Color * AmbientLight.Intensity * Material.AmbientColor.rgb;
    
    // Directional Light (diffuse)
    float3 dirLightDir = normalize(-DirectionalLight.Direction);
    float diffuseFactor = saturate(dot(normal, dirLightDir));
    float3 directionalDiffuse = DirectionalLight.Color * DirectionalLight.Intensity * Material.DiffuseColor.rgb * diffuseFactor;

#if LIGHTING_MODEL == LIGHTING_MODEL_LAMBERT
    // Lambert : Ambient + Diffuse
    float3 lighting = ambient + directionalDiffuse;

    // PointLights (diffuse)
    for (int i=0; i<NUM_POINT_LIGHT; i++)
    {
        float3 toPoint = PointLights[i].Position - Input.worldPos;
        float dist = length(toPoint);

        if (dist > PointLights[i].m_fAttRadius)
            continue;

        float3 L = normalize(toPoint);
        float diff = saturate(dot(normal, L));

        // 간단한 감쇠 : 1/(1 + att * dist^2)
        float attenuation = 1.0 / (1.0 + PointLights[i].m_fAttenuation * dist * dist);
        lighting += PointLights[i].DiffuseColor * PointLights[i].Intensity * Material.DiffuseColor.rgb * diff * attenuation;
    }

    // SpotLights (diffuse)
    [unroll]
    for (int i=0; i<NUM_SPOT_LIGHT; i++)
    {
        float3 toSpot = SpotLights[i].Position - Input.worldPos;
        float dist = length(toSpot);

        if (dist > SpotLights[i].m_fAttRadius)
            continue;

        float3 L = normalize(toSpot);
        float diff = saturate(dot(normal, L));

        // Spot Factor : 각도에 따른 Falloff
        float spotFactor = pow(saturate(dot(-L, normalize(SpotLights[i].Direction))), SpotLights[i].m_fFalloff);
        float attenuation = 1.0 / (1.0 + SpotLights[i].m_fAttenuation * dist * dist);
        lighting += SpotLights[i].DiffuseColor * SpotLights[i].Intensity * Material.DiffuseColor.rgb * diff * attenuation * spotFactor;
    }
    
    finalColor = baseColor * lighting + emissive;

#elif LIGHTING_MODEL == LIGHTING_MODEL_BLINNPHONG
    // Blinn-Phong : Ambient + Diffuse + Specular
    
    float3 viewDir = normalize(CameraPosition - Input.worldPos);
    
    // Directional light
    float3 halfVector = normalize(dirLightDir + viewDir);
    float specAngle = saturate(dot(normal, halfVector));
    float specular = pow(specAngle, 32.0); // 스페큘러 지수 32.0 (필요시 재질 값 사용)
    float3 specularDir = DirectionalLight.Color * DirectionalLight.Intensity * Material.SpecularColor.rgb * specular * Material.SpecularScalar;
    
    float3 lighting = ambient + directionalDiffuse + specularDir;
    
    // PointLights (diffuse + specular)
    [unroll]
    for (int i = 0; i < NUM_POINT_LIGHT; i++)
    {
        float3 toLight = PointLights[i].Position - Input.worldPos;
        float dist = length(toLight);
        if (dist > PointLights[i].m_fAttRadius)
            continue;
        float3 L = normalize(toLight);
        float diff = saturate(dot(normal, L));
        float attenuation = 1.0 / (1.0 + PointLights[i].m_fAttenuation * dist * dist);
        float3 diffuseP = PointLights[i].DiffuseColor * PointLights[i].Intensity * Material.DiffuseColor.rgb * diff * attenuation;
        
        float3 halfVec = normalize(L + viewDir);
        float specP = pow(saturate(dot(normal, halfVec)), 32.0);
        float3 specularP = PointLights[i].SpecularColor * PointLights[i].Intensity * Material.SpecularColor.rgb * specP * Material.SpecularScalar * attenuation;
        
        lighting += diffuseP + specularP;
    }
    
    // SpotLights (diffuse + specular)
    [unroll]
    for (int i = 0; i < NUM_SPOT_LIGHT; i++)
    {
        float3 toLight = SpotLights[i].Position - Input.worldPos;
        float dist = length(toLight);
        if (dist > SpotLights[i].m_fAttRadius)
            continue;
        float3 L = normalize(toLight);
        float diff = saturate(dot(normal, L));
        float spotFactor = pow(saturate(dot(-L, normalize(SpotLights[i].Direction))), SpotLights[i].m_fFalloff);
        float attenuation = 1.0 / (1.0 + SpotLights[i].m_fAttenuation * dist * dist);
        float3 diffuseS = SpotLights[i].DiffuseColor * SpotLights[i].Intensity * Material.DiffuseColor.rgb * diff * attenuation * spotFactor;
        
        float3 halfVec = normalize(L + viewDir);
        float specS = pow(saturate(dot(normal, halfVec)), 32.0);
        float3 specularS = SpotLights[i].SpecularColor * SpotLights[i].Intensity * Material.SpecularColor.rgb * specS * Material.SpecularScalar * attenuation * spotFactor;
        
        lighting += diffuseS + specularS;
    }
    
    finalColor = baseColor * lighting + emissive;

#elif LIGHTING_MODEL == LIGHTING_MODEL_UNLIT
    return float4(baseColor, 1.0f);
#endif
    return float4(finalColor, 1.0f);
}
#endif
