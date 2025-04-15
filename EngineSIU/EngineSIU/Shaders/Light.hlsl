// light.hlsl


#define MAX_LIGHTS 16 

#define POINT_LIGHT         1
#define SPOT_LIGHT          2

struct LIGHT
{
    float3 m_cDiffuse;
    float pad2;

    float3 m_cSpecular;
    float pad3;

    float3 m_vPosition;
    float m_fFalloff; // 스팟라이트의 감쇠 인자

    float3 m_vDirection;
    float pad4;

    float m_fAttenuation; // 거리 기반 감쇠 계수
    int m_bEnable;
    int m_nType;
    float m_fIntensity; // 광원 강도
    
    float m_fAttRadius; // 감쇠 반경 (Attenuation Radius)
    float3 LightPad;
    
    //float m_fInnerConeAngle; // 내부 원뿔 각도
    //float m_fOuterConeAngle; // 외부 원뿔 각도
    //float2 Pad1;
};

cbuffer cbLights : register(b2)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    int gnLights;
    float3 padCB;
};

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal)
{
    float3 vToLightSource = gLights[nIndex].m_vPosition - vPosition;
    float fDistanceToLight = length(vToLightSource);
    float3 vDirectionToLightSource = vToLightSource / fDistanceToLight;
    float3 normal = normalize(vNormal);

    float fSpotCosAngle =  dot(-vDirectionToLightSource, gLights[nIndex].m_vDirection); // length(gLights[nIndex].m_vDirection);
    
    
    float DistanceAttenuation = saturate(1.0f - fDistanceToLight / (gLights[nIndex].m_fAttRadius));
    
    //float CosInnerConeAngle = cos(radians(gLights[nIndex].m_fInnerConeAngle));
    //float CosOuterConeAngle = cos(radians(gLights[nIndex].m_fOuterConeAngle));
    float cosInnerConeAngle = cos(radians(15.0f/2)); // 예시: 내부 원뿔 반각 30도의 코사인
    float cosOuterConeAngle = cos(radians(30.0f/2)); // 예시: 외부 원뿔 반각 60도의 코사인
    
    float SpotLightRatioBase = (fSpotCosAngle - cosOuterConeAngle) / (cosInnerConeAngle - cosOuterConeAngle);
    //float SpotLightRatioBase = (fSpotCosAngle - cosInnerConeAngle) / (cosInnerConeAngle - cosOuterConeAngle);
    float SaturatedSpotLightRatioBase = saturate(SpotLightRatioBase);
    
    float SpotFalloffExponent = gLights[nIndex].m_fFalloff;
    float SpotAngleAttenuation = pow(SaturatedSpotLightRatioBase, SpotFalloffExponent);
    
    // Lambertian Diffuse 
    float3 DiffuseColorRGB = gLights[nIndex].m_cDiffuse;
    float LambertFactor = max(0.0f, dot(normal, vDirectionToLightSource));
    float3 DiffuseLightContribution = DiffuseColorRGB * LambertFactor;
    
    float CombinedAttenuation = DistanceAttenuation * SpotAngleAttenuation;

    // 최종 확산 색상 (감쇠 및 광원 강도 적용)
    float3 FinalDiffuseColor = DiffuseLightContribution * CombinedAttenuation * gLights[nIndex].m_fIntensity;
    
    float4 FinalColor = float4(FinalDiffuseColor, 1.0f);

    return FinalColor;
}
    /*
    // 그리드 시뮬레이션 계산
    float4 GridColor = float4(1.0f, 1.0f, 0.0f, 1.0f); //노란색 그리드

    float LineThickness = 0.05f;
    bool IsNearGridLine = false;
    
    float3 LightSourcePosition = gLights[nIndex].m_vPosition; // 스포트라이트 시작 위치
    float3 LightDirection = normalize(gLights[nIndex].m_vDirection); // 스포트라이트가 향하는 방향 (단위 벡터)
    float MaxRadius = gLights[nIndex].m_fAttRadius; // 빛이 최대로 닿는 거리
    
    // 외부 원뿔 각도 설정. 
    float OuterConeAngleDegrees = 30.0f;
    float OuterConeHalfAngleRadians = radians(OuterConeAngleDegrees / 2.0f); 
    
    // 원뿔 밑면 원 기하학 정보 계산
    float CosOuterHalfAngle = cos(OuterConeHalfAngleRadians);
    float SinOuterHalfAngle = sin(OuterConeHalfAngleRadians);
    
    float ConeHeight = MaxRadius * CosOuterHalfAngle; // 원뿔의 높이 
    float BaseRadius = MaxRadius * SinOuterHalfAngle; // 밑면 원의 반지름
    
    float3 BaseCenterPosition = LightSourcePosition * LightDirection * ConeHeight; // 스포트라이트 위치에서 빛 방향으로 ConeHeight만큼 이동하면 원의 중심 위치
    
    // 현재 픽셀이 원 테두리 근처인지 확인. 
    float DistanceToPlane = abs(dot(vPosition - BaseCenterPosition, LightDirection));
    
    // 5b. 픽셀을 원이 놓인 평면에 투영(수직으로 내림)했을 때, 그 점이 원 중심에서 얼마나 떨어져 있는지 계산
    float3 ProjectedPoint = vPosition - (DistanceToPlane * LightDirection); // 평면 위의 점으로 투영 
    float DistanceFromCenterOnPlane = length(ProjectedPoint - BaseCenterPosition);
    
    if (DistanceToPlane < LineThickness && abs(DistanceFromCenterOnPlane - BaseRadius) < LineThickness)
    {
        IsNearGridLine = true; // 픽셀이 원 테두리 선 근처에 있다고 표시!
    }
    
    // --- 최종 색상 결정 ---
    float4 FinalColor = OriginalColor; // 일단 원래 색깔로 시작
    if (IsNearGridLine)
    {
        FinalColor = GridColor; // 만약 선 근처라면 노란색으로 덮어쓰기!
    }
    */


float4 PointLight(int nIndex, float3 vPosition, float3 vNormal)
{
    // 광원과 픽셀 위치 간 벡터 계산
    float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
    float fDistance = length(vToLight);

    // 감쇠 반경을 벗어나면 기여하지 않음
    if (fDistance > gLights[nIndex].m_fAttRadius)
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
        fSpecularFactor = pow(max(dot(normalize(vNormal), vHalf), 0.0f), 1);
    }

    float fAttenuationFactor = 1.0f / (1.0f + gLights[nIndex].m_fAttenuation * fDistance * fDistance);
   
    float3 lit = (gcGlobalAmbientLight * Material.AmbientColor.rgb) +
                 (gLights[nIndex].m_cDiffuse.rgb * fDiffuseFactor * Material.DiffuseColor) +
                 (gLights[nIndex].m_cSpecular.rgb * fSpecularFactor * Material.SpecularColor);

    return float4(lit * fAttenuationFactor * gLights[nIndex].m_fIntensity, 1.0f);
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [unroll(MAX_LIGHTS)]
    for (int i = 0; i < gnLights; i++)
    {
        if (gLights[i].m_bEnable)
        {
            if (gLights[i].m_nType == POINT_LIGHT)
            {
                cColor += PointLight(i, vPosition, vNormal);
            }
            else if (gLights[i].m_nType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, vPosition, vNormal);
            }
        }
    }
    
    // 전역 환경광 추가
    cColor += gcGlobalAmbientLight;
    cColor.a = 1;
    
    return cColor;
}






