
#include "GizmoRenderPass.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "RendererHelpers.h"
#include "Math/JungleMath.h"

#include "World/World.h"

#include "Actors/Player.h"

#include "BaseGizmos/GizmoBaseComponent.h"
#include "BaseGizmos/TransformGizmo.h"

#include "UnrealEd/EditorViewportClient.h"

#include "PropertyEditor/ShowFlags.h"

#include "EngineLoop.h"

#include "UObject/ObjectTypes.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/EditorEngine.h"


// 생성자/소멸자
FGizmoRenderPass::FGizmoRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FGizmoRenderPass::~FGizmoRenderPass()
{
    ReleaseShader();
    if (ShaderManager)
    {
        delete ShaderManager;
        ShaderManager = nullptr;
    }
}

void FGizmoRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
    CreateShader();
}

void FGizmoRenderPass::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC GizmoInputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Stride = sizeof(FStaticMeshVertex);

    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"GizmoVertexShader", L"Shaders/GizmoVertexShader.hlsl", "mainVS", GizmoInputLayout, ARRAYSIZE(GizmoInputLayout));

    hr = ShaderManager->AddPixelShader(L"GizmoPixelShader", L"Shaders/GizmoPixelShader.hlsl", "mainPS");

    ReloadShader();

    InputLayout = ShaderManager->GetInputLayoutByKey(L"GizmoVertexShader");

}
void FGizmoRenderPass::ReleaseShader()
{
    FDXDBufferManager::SafeRelease(InputLayout);
    FDXDBufferManager::SafeRelease(PixelShader);
    FDXDBufferManager::SafeRelease(VertexShader);
}


void FGizmoRenderPass::ClearRenderArr()
{
    //GizmoObjs.Empty();
}

void FGizmoRenderPass::ReloadShader()
{
    VertexShader = ShaderManager->GetVertexShaderByKey(L"GizmoVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"GizmoPixelShader");
}

void FGizmoRenderPass::PrepareRenderState() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    // 상수 버퍼 바인딩 예시
    ID3D11Buffer* PerObjectBuffer = BufferManager->GetConstantBuffer(TEXT("FPerObjectConstantBuffer"));
    ID3D11Buffer* CameraConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FCameraConstantBuffer"));
    Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &PerObjectBuffer);
    Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &CameraConstantBuffer);

    TArray<FString> PSBufferKeys = {
                                  TEXT("FPerObjectConstantBuffer"),
                                   TEXT("FMaterialConstants"),
                                  TEXT("FLitUnlitConstants")
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);
}

void FGizmoRenderPass::PrepareRender()
{
    /*for (const auto iter : TObjectRange<UGizmoBaseComponent>())
    {
        GizmoObjs.Add(iter);
    }*/
}

void FGizmoRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    if (GEngine->ActiveWorld->WorldType != EWorldType::Editor)
        return;
    
    PrepareRenderState();
    Graphics->DeviceContext->ClearDepthStencilView(Graphics->DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        UE_LOG(LogLevel::Error, TEXT("Gizmo RenderPass : Render : Engine is not valid."));
        return;
    }
    ControlMode Mode = Engine->GetEditorPlayer()->GetControlMode();
    UWorld* ActiveWorld = GEngine->ActiveWorld;
    if (!ActiveWorld)
    {
        UE_LOG(LogLevel::Error, TEXT("Gizmo RenderPass : Render : ActiveWorld is not valid."));
        return;
    }

    if (Mode == CM_TRANSLATION)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetArrowArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);
            Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerStateSOLID);

            RenderGizmoComponent(GizmoComp, Viewport, ActiveWorld);

            Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());
            Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
        }
    }
    else if (Mode == CM_SCALE)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetScaleArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);

            Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerStateSOLID);

            RenderGizmoComponent(GizmoComp, Viewport, ActiveWorld);

            Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());
            Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
        }
    }
    else if (Mode == CM_ROTATION)
    {
        for (UStaticMeshComponent* StaticMeshComp : Viewport->GetGizmoActor()->GetDiscArr())
        {
            UGizmoBaseComponent* GizmoComp = Cast<UGizmoBaseComponent>(StaticMeshComp);
            Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerStateSOLID);
            Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerStateSOLID);

            RenderGizmoComponent(GizmoComp, Viewport, ActiveWorld);

            Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());
            Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
        }
    }
}

void FGizmoRenderPass::RenderGizmoComponent(UGizmoBaseComponent* GizmoComp, const std::shared_ptr<FEditorViewportClient>& Viewport, const UWorld* World)
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (Engine && !Engine->GetSelectedActor())
        return;
    // 모델 행렬.
    FMatrix Model = GizmoComp->GetWorldMatrix();

    FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;

    bool Selected = (GizmoComp == Viewport->GetPickedGizmoComponent());

    FMatrix NormalMatrix = RendererHelpers::CalculateNormalMatrix(Model);

    FPerObjectConstantBuffer Data(Model, NormalMatrix, UUIDColor, Selected);

    FCameraConstantBuffer CameraData(Viewport->View, Viewport->Projection);

    BufferManager->UpdateConstantBuffer(TEXT("FPerObjectConstantBuffer"), Data);
    BufferManager->UpdateConstantBuffer(TEXT("FCameraConstantBuffer"), CameraData);

    // Gizmo가 렌더링할 StaticMesh가 없으면 렌더링하지 않음
    if (!GizmoComp->GetStaticMesh())
        return;

    OBJ::FStaticMeshRenderData* RenderData = GizmoComp->GetStaticMesh()->GetRenderData();

    if (!RenderData)
        return;

    UINT stride = sizeof(FStaticMeshVertex);

    UINT offset = 0;

    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &stride, &offset);

    if (RenderData->IndexBuffer)
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
    }
    else
    {
        for (int subMeshIndex = 0; subMeshIndex < RenderData->MaterialSubsets.Num(); subMeshIndex++)
        {
            int materialIndex = RenderData->MaterialSubsets[subMeshIndex].MaterialIndex;

            FSubMeshConstants SubMeshData = FSubMeshConstants(false);
            BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

            TArray<FStaticMaterial*>Materials = GizmoComp->GetStaticMesh()->GetMaterials();
            TArray<UMaterial*>OverrideMaterials = GizmoComp->GetOverrideMaterials();

            if (OverrideMaterials[materialIndex] != nullptr)
                MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[materialIndex]->GetMaterialInfo());
            else
                MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[materialIndex]->Material->GetMaterialInfo());

            uint64 startIndex = RenderData->MaterialSubsets[subMeshIndex].IndexStart;
            uint64 indexCount = RenderData->MaterialSubsets[subMeshIndex].IndexCount;

            Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
        }
    }
}
