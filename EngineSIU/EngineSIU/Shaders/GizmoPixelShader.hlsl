// GizmoPixelShader.hlsl

#include <Common.hlsl>

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer FlagConstants : register(b2)
{
    bool IsLit;
    float3 flagPad0;
}

struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치
    float4 color : COLOR; // 전달된 베이스 컬러
    float3 normal : NORMAL; // 월드 공간 노멀
    float2 texcoord : TEXCOORD1; // UV 좌표
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
    
    float3 matDiffuse = Material.DiffuseColor.rgb;
    
    output.color = float4(matDiffuse, 1);
    if (isSelected)
    {
        output.color += float4(0.5f, 0.5f, 0.5f, 1); // 선택된 경우 빨간색으로 설정
    }
      
    return output;
}
