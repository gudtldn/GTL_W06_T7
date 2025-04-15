#include "Define.h"
#include "UObject/Casts.h"
#include "UpdateLightBufferPass.h"

#include "Components/Light/AmbientLightComponent.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"

#include "UObject/UObjectIterator.h"

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------
FUpdateLightBufferPass::FUpdateLightBufferPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FUpdateLightBufferPass::~FUpdateLightBufferPass()
{
}

void FUpdateLightBufferPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FUpdateLightBufferPass::PrepareRender()
{
    for (const auto iter : TObjectRange<ULightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            // Todo : 추후 Global Light 추가
            if (UAmbientLightComponent* AmbientLight = Cast<UAmbientLightComponent>(iter))
            {
                this->AmbientLight = AmbientLight;
            }
            if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
            {
                PointLights.Add(PointLight);
            }
            else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
            {
                SpotLights.Add(SpotLight);
            }
        }
    }
}

void FUpdateLightBufferPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FLighting LightBufferData = {};

    // ToDo : AmbientLight, DirectionalLight 추후 생성해 세팅
    FAmbientLightInfo AmbientLight = {};
    AmbientLight.Color = FVector(0.2f, 0.2f, 0.25f);  // 약간 차가운 느낌의 기본 색상
    AmbientLight.Intensity = 1.0f;                    // 기본 강도
    LightBufferData.AmbientLight = AmbientLight;

    FDirectionalLightInfo DirectionalLight = {};
    DirectionalLight.Direction = FVector(-1.0f, -0.7f, -0.5f);  // 빛의 진행 방향 (필요에 따라 정규화할 수도 있음)
    DirectionalLight.Color = FVector(1.0f, 1.0f, 0.95f);         // 약간 따뜻한 백색광
    DirectionalLight.Intensity = 1.5f;
    LightBufferData.DirectionalLight = DirectionalLight;
    
    int pointLightIndex = 0;
    for (auto Light : PointLights)
    {
        if (pointLightIndex < MAX_LIGHTS)
        {
            FPointLightInfo PointLight = {};
            PointLight.Position = Light->GetWorldLocation();
            FLinearColor DiffuseColor = Light->GetDiffuseColor();
            PointLight.DiffuseColor = FVector(DiffuseColor.R, DiffuseColor.G, DiffuseColor.B);
            FLinearColor SpecularColor = Light->GetSpecularColor();
            PointLight.SpecularColor = FVector(SpecularColor.R, SpecularColor.G, SpecularColor.B);
            PointLight.Intensity = Light->GetIntensity();
            PointLight.m_fAttRadius = Light->GetAttenuationRadius();
            PointLight.m_fAttenuation = Light->GetAttenuation();

            LightBufferData.PointLights[pointLightIndex] = PointLight;
            ++pointLightIndex;
        }
    }

    int spotLightIndex = 0;
    for (auto Light : SpotLights)
    {
        if (spotLightIndex < MAX_LIGHTS)
        {
            FSpotLightInfo SpotLight = {};
            SpotLight.Position = Light->GetWorldLocation();
            SpotLight.Direction = Light->GetForwardVector();
            FLinearColor DiffuseColor = Light->GetDiffuseColor();
            SpotLight.DiffuseColor = FVector(DiffuseColor.R, DiffuseColor.G, DiffuseColor.B);
            FLinearColor SpecularColor = Light->GetSpecularColor();
            SpotLight.SpecularColor = FVector(SpecularColor.R, SpecularColor.G, SpecularColor.B);
            SpotLight.Intensity = Light->GetIntensity();
            SpotLight.m_fAttRadius = Light->GetAttenuationRadius();
            SpotLight.m_fFalloff = Light->GetFalloff();
            SpotLight.m_fAttenuation = Light->GetAttenuation();
            
            //// 월드 변환 행렬 계산 (스케일 1로 가정)
            //FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), { 1, 1, 1 });
            //FEngineLoop::PrimitiveDrawBatch.AddConeToBatch(Light->GetWorldLocation(), 100, Light->GetRange(), 140, {1,1,1,1}, Model);
            //FEngineLoop::PrimitiveDrawBatch.AddOBBToBatch(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
            LightBufferData.SpotLights[spotLightIndex] = SpotLight;

            spotLightIndex++;
        }
    }
    BufferManager->UpdateConstantBuffer(TEXT("FLightBuffer"), LightBufferData);
}

void FUpdateLightBufferPass::ClearRenderArr()
{
    PointLights.Empty();
    SpotLights.Empty();
}

void FUpdateLightBufferPass::UpdateLightBuffer(FLighting Light) const
{

}
