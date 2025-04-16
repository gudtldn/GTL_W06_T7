// staticMeshPixelShader.hlsl
#define PIXEL_SHADER

Texture2D Textures : register(t0);
Texture2D BumpTexture : register(t1);
SamplerState Sampler : register(s0);
SamplerState BumpSampler : register(s1);

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};
cbuffer CameraConstants : register(b1)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float3 CameraPosition;
    float pad;
};

//struct FMaterial
//{
//    float3 DiffuseColor;
//    float TransparencyScalar;
    
//    float3 AmbientColor;
//    float DensityScalar;
    
//    float3 SpecularColor;
//    float SpecularScalar;
    
//    float3 EmissiveColor;
//    int bUseBumpMap;
//};
//cbuffer MaterialConstants : register(b3)
//{
//    FMaterial Material;
//}
cbuffer FlagConstants : register(b4)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b5)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b6)
{
    float2 UVOffset;
    float2 TexturePad0;
}

cbuffer RenderNormalConstants : register(b7)
{
    bool IsRenderNormal;
    float3 RenderNormalPad0;
}

#include "UberLit.hlsl"

struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치
    float4 color : COLOR; // 전달된 베이스 컬러
    float3 normal : NORMAL; // 월드 공간 노멀
    float3 tangent : TANGENT; // 월드 공간 탄젠트
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그
    float2 texcoord : TEXCOORD2; // UV 좌표
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
};




PS_OUTPUT mainPS(PS_INPUT input)
{
    
    PS_OUTPUT output;
    output.UUID = UUID;

    float3 Normal;
    
    // BumpMap 샘플링
    if (Material.bUseBumpMap == 1)
    {
    //    Normal = BumpTexture.Sample(BumpSampler, input.texcoord).rgb;
    //    //output.color = float4(Normal, 1);
    //    //return output;
    //    // 샘플링된 노멀을 [-1, 1] 범위로 변환
    //    Normal = normalize(Normal * 2.0f - 1.0f);
        
    //    // N : 보간된 버텍스 노멀
    //    float3 N = normalize(input.normal);
    //    // T : 보간된 버텍스 탄젠트
    //    float3 T = normalize(input.tangent);
    //    //float3 T = normalize(normalize(input.tangent) - dot(input.tangent, N) * N);
    //    // B : N과 T의 외적
    //    //float3 B = normalize(cross(T, N));
    //    float3 B = normalize(cross(N, T));
        
    //    // TBN 변환 행렬 생성
    //    // 탄젠트 공간 벡터를 월드 공간으로 변환하기 위해
    //    float3x3 TBN = float3x3(T, B, N);
    //    //TBN = transpose(TBN);
  
        
    //    // 샘플링된 노멀을 TBN 행렬을 이용해 월드 공간으로 변환
    //    Normal = normalize(mul(Normal, TBN));
    //    //Normal = normalize(mul(mul(Normal, TBN), (float3x3) Model));
    //    //Normal.y *= -1;
    //    //output.color = float4(Normal, 1.f);
    //    //return output;
        
        
        // 1) 범프 맵 샘플링
        float3 sampledNormal = BumpTexture.Sample(BumpSampler, input.texcoord).rgb;
        
        
        //output.color = float4(sampledNormal, 1.f);
        //return output;
        
        // 2) 범위 변환 [0,1] -> [-1,1]
        sampledNormal = normalize(sampledNormal * 2.0f - 1.0f);
        
        // 3) 보간된 노멀/탄젠트/비탄젠트 벡터 계산
        float3 N = normalize(input.normal);
        float3 T = normalize(input.tangent);
        T = normalize(T - dot(T, N) * N);
        float3 B = cross(N, T);
        
        float3x3 TBN = float3x3(T, B, N);
        Normal = normalize(mul(sampledNormal, TBN));
        //float3 normalColor = normalWS * 0.5f + 0.5f;
        //output.color = float4(Normal * 0.5f + 0.5f, 1.0f);
        //return output;
        
        //// 4) TBN 행렬 구성
        //float3x3 TBN = float3x3(T, B, N);

        //// 5) 월드 공간 노멀로 변환
        //float3 worldNormal = mul(sampledNormal, TBN);
        //worldNormal = mul(worldNormal, (float3x3) Model);
        //worldNormal = normalize(worldNormal);

        //// 6) 시각화용으로 [-1,1] 범위를 [0,1]로 매핑
        //float3 debugColor = (worldNormal * 0.5f) + 0.5f;

        //// 7) 픽셀 출력
        //output.color = outputColor = float4(debugColor, 1.0f);

        
        //return output;
    }
    else // bUseBumpMap == 0
    {
        Normal = normalize(input.normal);
    }
    
    if (IsRenderNormal)
    {
        output.color = float4(Normal * 0.5f + 0.5f, 1.0f);
        return output;
    }

#if LIGHTING_MODEL == LIGHTING_MODEL_GOURAUD
    output.color = input.color;
#else
    PixelInput uberPsInput;
    uberPsInput.position = input.position;
    uberPsInput.worldPos = input.worldPos;
    uberPsInput.normal   = Normal;
    uberPsInput.texcoord = input.texcoord;
    uberPsInput.color    = input.color;
    
    output.color = Uber_PS(uberPsInput);
#endif
    if (isSelected)
    {
        output.color += float4(0.02, 0.02, 0.02, 1);
    }
    
    return output;
}
