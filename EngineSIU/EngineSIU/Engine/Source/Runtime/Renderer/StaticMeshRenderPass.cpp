#include "StaticMeshRenderPass.h"

#include "EngineLoop.h"
#include "World/World.h"

#include "RendererHelpers.h"
#include "Math/JungleMath.h"

#include "UObject/UObjectIterator.h"
#include "UObject/Casts.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "Components/StaticMeshComponent.h"

#include "BaseGizmos/GizmoBaseComponent.h"
#include "Engine/EditorEngine.h"

#include "PropertyEditor/ShowFlags.h"

#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

FStaticMeshRenderPass::FStaticMeshRenderPass()
    : VertexShader(nullptr)
    , PixelShader(nullptr)
    , InputLayout(nullptr)
    , Stride(0)
    , BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FStaticMeshRenderPass::~FStaticMeshRenderPass()
{
    ReleaseShader();
    if (ShaderManager)
    {
        delete ShaderManager;
        ShaderManager = nullptr;
    }
}

void FStaticMeshRenderPass::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC StaticMeshLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    D3D11_INPUT_ELEMENT_DESC TextureLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Stride = sizeof(FStaticMeshVertex);

    ShaderManager->RegisterShaderVariants();
    
    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"StaticMeshVertexShader", L"Shaders/StaticMeshVertexShader.hlsl", "mainVS", StaticMeshLayoutDesc, ARRAYSIZE(StaticMeshLayoutDesc));

    //VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");

    ReloadShader();

    InputLayout = ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
    // PixelShader = ShaderManager->GetPixelShaderByKey(L"BlinnPhong");  // Default Pixel Shader
}
void FStaticMeshRenderPass::ReleaseShader()
{
    FDXDBufferManager::SafeRelease(InputLayout);
    FDXDBufferManager::SafeRelease(PixelShader);
    FDXDBufferManager::SafeRelease(VertexShader);
}

void FStaticMeshRenderPass::ChangeViewMode(EViewModeIndex evi) const
{
    const_cast<FStaticMeshRenderPass*>(this)->UpdateShadersByViewMode(evi);

    switch (evi)
    {
    case EViewModeIndex::VMI_Lit_Gouraud:
    case EViewModeIndex::VMI_Lit_Lambert:
    case EViewModeIndex::VMI_Lit_Phong:
        UpdateLitUnlitConstant(1);
        UpdateRenderNormalConstant(0);
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        UpdateLitUnlitConstant(0);
        UpdateRenderNormalConstant(0);
        break;
    case EViewModeIndex::VMI_Normal:
        UpdateRenderNormalConstant(1);
        break;
    }
}


void FStaticMeshRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShader();
}

void FStaticMeshRenderPass::PrepareRender()
{
    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (!Cast<UGizmoBaseComponent>(iter) && iter->GetWorld() == GEngine->ActiveWorld)
        {
            StaticMeshObjs.Add(iter);
        }
    }
}

void FStaticMeshRenderPass::PrepareRenderState() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    // 상수 버퍼 바인딩 예시
    // ID3D11Buffer* PerObjectBuffer = BufferManager->GetConstantBuffer(TEXT("FPerObjectConstantBuffer"));
    // ID3D11Buffer* CameraConstantBuffer = BufferManager->GetConstantBuffer(TEXT("FCameraConstantBuffer"));
    // Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &PerObjectBuffer);
    // Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &CameraConstantBuffer);

    TArray<FString> VSBufferKeys = {TEXT("FPerObjectConstantBuffer"),
                                  TEXT("FCameraConstantBuffer"),
                                  TEXT("FLightBuffer"),
                                    TEXT("FMaterialConstants")
    };
    BufferManager->BindConstantBuffers(VSBufferKeys, 0, EShaderStage::Vertex);

    TArray<FString> PSBufferKeys = {
                                  TEXT("FCameraConstantBuffer"),
                                  TEXT("FLightBuffer"),
                                  TEXT("FMaterialConstants"),
                                  TEXT("FLitUnlitConstants"),
                                  TEXT("FSubMeshConstants"),
                                  TEXT("FTextureConstants"),
                                  TEXT("FRenderNormalConstants")
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 1, EShaderStage::Pixel);
}

void FStaticMeshRenderPass::UpdatePerObjectConstant(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection, const FVector4& UUIDColor, bool Selected) const
{
    FMatrix NormalMatrix = RendererHelpers::CalculateNormalMatrix(Model);
    FPerObjectConstantBuffer Data(Model, NormalMatrix, UUIDColor, Selected);
    BufferManager->UpdateConstantBuffer(TEXT("FPerObjectConstantBuffer"), Data);
   
}

void FStaticMeshRenderPass::UpdateLitUnlitConstant(int isLit) const
{
    FLitUnlitConstants Data;
    Data.isLit = isLit;
    BufferManager->UpdateConstantBuffer(TEXT("FLitUnlitConstants"), Data);
}

void FStaticMeshRenderPass::UpdateRenderNormalConstant(bool bRenderNormal) const
{
    FRenderNormalConstants Data = FRenderNormalConstants(false, FVector(0.f, 0.f, 0.f));
    Data.bRenderNormal = bRenderNormal;
    BufferManager->UpdateConstantBuffer(TEXT("FRenderNormalConstants"), Data);
}


void FStaticMeshRenderPass::RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData, TArray<FStaticMaterial*> Materials, TArray<UMaterial*> OverrideMaterials, int SelectedSubMeshIndex) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &offset);
    if (RenderData->IndexBuffer)
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (RenderData->MaterialSubsets.Num() == 0) {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        return;
    }

    for (int subMeshIndex = 0; subMeshIndex < RenderData->MaterialSubsets.Num(); subMeshIndex++) {

        int materialIndex = RenderData->MaterialSubsets[subMeshIndex].MaterialIndex;

        FSubMeshConstants SubMeshData = (subMeshIndex == SelectedSubMeshIndex) ? FSubMeshConstants(true) : FSubMeshConstants(false);

        BufferManager->UpdateConstantBuffer(TEXT("FSubMeshConstants"), SubMeshData);

        if (OverrideMaterials[materialIndex] != nullptr)
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, OverrideMaterials[materialIndex]->GetMaterialInfo());
        else
            MaterialUtils::UpdateMaterial(BufferManager, Graphics, Materials[materialIndex]->Material->GetMaterialInfo());

        uint64 startIndex = RenderData->MaterialSubsets[subMeshIndex].IndexStart;
        uint64 indexCount = RenderData->MaterialSubsets[subMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
    }
}


void FStaticMeshRenderPass::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FStaticMeshRenderPass::RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FStaticMeshRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    if (!(Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))) return;

    PrepareRenderState();

    for (UStaticMeshComponent* Comp : StaticMeshObjs) {
        if (!Comp || !Comp->GetStaticMesh()) continue;
        
        FMatrix Model = Comp->GetWorldMatrix();
        FVector4 UUIDColor = Comp->EncodeUUID() / 255.0f;
        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
        bool Selected = (Engine && Engine->GetSelectedActor() == Comp->GetOwner());

        UpdatePerObjectConstant(Model, Viewport->GetViewMatrix(), Viewport->GetProjectionMatrix(), UUIDColor, Selected);
        FCameraConstantBuffer CameraData(Viewport->GetViewMatrix(), Viewport->GetProjectionMatrix(), Viewport->ViewTransformPerspective.GetLocation(), 0);
        BufferManager->UpdateConstantBuffer(TEXT("FCameraConstantBuffer"), CameraData);

        OBJ::FStaticMeshRenderData* RenderData = Comp->GetStaticMesh()->GetRenderData();

        if (RenderData == nullptr) continue;

        RenderPrimitive(RenderData, Comp->GetStaticMesh()->GetMaterials(), Comp->GetOverrideMaterials(), Comp->GetselectedSubMeshIndex());

        if (Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            FEngineLoop::PrimitiveDrawBatch.AddAABBToBatch(Comp->GetBoundingBox(), Comp->GetWorldLocation(), Model);
        }
    }
}

void FStaticMeshRenderPass::ClearRenderArr()
{
    StaticMeshObjs.Empty();
}

void FStaticMeshRenderPass::UpdateShadersByViewMode(EViewModeIndex evi)
{
    ELightingModel LightingModel;

    switch (evi)
    {
    case EViewModeIndex::VMI_Lit_Gouraud:
        LightingModel = ELightingModel::Gouraud;
        break;
    case EViewModeIndex::VMI_Lit_Lambert:
        LightingModel = ELightingModel::Lambert;
        break;
    case EViewModeIndex::VMI_Lit_Phong:
        LightingModel = ELightingModel::BlinnPhong;
        break;
    case EViewModeIndex::VMI_Unlit:
    default:
        LightingModel = ELightingModel::Unlit;
        break;
    }

    FShaderPipeline Pipeline = ShaderManager->GetShaderPipelineByLightingModel(LightingModel);

    VertexShader = Pipeline.VertexShader;
    PixelShader = Pipeline.PixelShader;
}

void FStaticMeshRenderPass::ReloadShader()
{
    VertexShader = ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"StaticMeshPixelShader");
}
