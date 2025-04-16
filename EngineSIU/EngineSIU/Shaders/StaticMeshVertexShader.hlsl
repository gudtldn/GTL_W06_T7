// StaticMeshVertexShader.hlsl
#include <Common.hlsl>

// MatrixBuffer: 변환 행렬 관리
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

#include "UberLit.hlsl"

struct VS_INPUT
{
    float3 position : POSITION; // 버텍스 위치
    float3 normal : NORMAL; // 버텍스 노멀
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR; // 버텍스 색상
    int materialIndex : MATERIAL_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // 클립 공간으로 변환된 화면 좌표
    float3 worldPos : TEXCOORD0; // 월드 공간 위치 (조명용)
    float4 color : COLOR; // Gouraud 조명 결과
    float3 normal : NORMAL; // 월드 공간 노멀
    float3 tangent : TANGENT; // 월드 공간 탄젠트
    float normalFlag : TEXCOORD1; // 노멀 유효 플래그 (1.0 또는 0.0)
    float2 texcoord : TEXCOORD2; // UV 좌표
    int materialIndex : MATERIAL_INDEX; // 머티리얼 인덱스
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    float4 worldPosition = mul(float4(input.position, 1), Model);
    float3 worldNormal = normalize(mul(input.normal, (float3x3) MInverseTranspose));
    float4 viewPosition = mul(worldPosition, View);
    
    output.position = mul(viewPosition, Projection);
    output.worldPos = worldPosition.xyz;
    output.normal = worldNormal;
    output.tangent = normalize(mul(input.tangent, (float3x3) Model));
    output.normalFlag = length(worldNormal) > 0.001f ? 1.0f : 0.0f;
    output.texcoord = input.texcoord;
    output.materialIndex = input.materialIndex;

    VertexInput lightInput;
    lightInput.worldPos = worldPosition.xyz;
    lightInput.normalWS = worldNormal;
    lightInput.color = float4(Material.DiffuseColor.rgb, 1.0f);
    output.color = Uber_VS(lightInput);
    
    return output;
}
