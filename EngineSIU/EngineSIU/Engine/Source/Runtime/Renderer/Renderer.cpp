
#include "Renderer.h"
#include "World/World.h"
#include "Engine/EditorEngine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "RendererHelpers.h"
#include "StaticMeshRenderPass.h"
#include "BillboardRenderPass.h"
#include "GizmoRenderPass.h"
#include "UpdateLightBufferPass.h"
#include "LineRenderPass.h"
#include "DepthBufferDebugPass.h"
#include "FogRenderPass.h"
#include "EditorRenderPass.h"
#include <UObject/UObjectIterator.h>
#include <UObject/Casts.h>
#include "GameFrameWork/Actor.h"

//------------------------------------------------------------------------------
// 초기화 및 해제 관련 함수
//------------------------------------------------------------------------------
void FRenderer::Initialize(FGraphicsDevice* InGraphics, FDXDBufferManager* InBufferManager)
{
    Graphics = InGraphics;
    BufferManager = InBufferManager;

    ShaderManager = new FDXDShaderManager(Graphics->Device);
    StaticMeshRenderPass = new FStaticMeshRenderPass();
    BillboardRenderPass = new FBillboardRenderPass();
    GizmoRenderPass = new FGizmoRenderPass();
    UpdateLightBufferPass = new FUpdateLightBufferPass();
    LineRenderPass = new FLineRenderPass();
    DepthBufferDebugPass = new FDepthBufferDebugPass();
    FogRenderPass = new FFogRenderPass();
    EditorRenderPass = new FEditorRenderPass();

    StaticMeshRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    BillboardRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    GizmoRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    UpdateLightBufferPass->Initialize(BufferManager, Graphics, ShaderManager);
    LineRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    DepthBufferDebugPass->Initialize(BufferManager, Graphics, ShaderManager);
    FogRenderPass->Initialize(BufferManager, Graphics, ShaderManager);
    EditorRenderPass->Initialize(BufferManager, Graphics, ShaderManager);

    CreateConstantBuffers();
}

void FRenderer::Release()
{
}


void FRenderer::ChangeViewMode(EViewModeIndex evi)
{
    StaticMeshRenderPass->ChangeViewMode(evi);
    if (evi == EViewModeIndex::VMI_SceneDepth)
        IsSceneDepth = true;
    else
        IsSceneDepth = false;
}

//------------------------------------------------------------------------------
// 사용하는 모든 상수 버퍼 생성
//------------------------------------------------------------------------------
void FRenderer::CreateConstantBuffers()
{
    UINT perObjectBufferSize = sizeof(FPerObjectConstantBuffer);
    BufferManager->CreateBufferGeneric<FPerObjectConstantBuffer>("FPerObjectConstantBuffer", nullptr, perObjectBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT cameraConstantBufferSize = sizeof(FCameraConstantBuffer);
    BufferManager->CreateBufferGeneric<FCameraConstantBuffer>("FCameraConstantBuffer", nullptr, cameraConstantBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT subUVBufferSize = sizeof(FSubUVConstant);
    BufferManager->CreateBufferGeneric<FSubUVConstant>("FSubUVConstant", nullptr, subUVBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT materialBufferSize = sizeof(FMaterialConstants);
    BufferManager->CreateBufferGeneric<FMaterialConstants>("FMaterialConstants", nullptr, materialBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT subMeshBufferSize = sizeof(FSubMeshConstants);
    BufferManager->CreateBufferGeneric<FSubMeshConstants>("FSubMeshConstants", nullptr, subMeshBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT textureBufferSize = sizeof(FTextureConstants);
    BufferManager->CreateBufferGeneric<FTextureConstants>("FTextureConstants", nullptr, textureBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT lightingBufferSize = sizeof(FLighting);
    BufferManager->CreateBufferGeneric<FLighting>("FLightBuffer", nullptr, lightingBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT litUnlitBufferSize = sizeof(FLitUnlitConstants);
    BufferManager->CreateBufferGeneric<FLitUnlitConstants>("FLitUnlitConstants", nullptr, litUnlitBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT ScreenConstantsBufferSize = sizeof(FScreenConstants);
    BufferManager->CreateBufferGeneric<FScreenConstants>("FScreenConstants", nullptr, ScreenConstantsBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    UINT FogConstantBufferSize = sizeof(FFogConstants);
    BufferManager->CreateBufferGeneric<FFogConstants>("FFogConstants", nullptr, FogConstantBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void FRenderer::ReleaseConstantBuffer()
{
    BufferManager->ReleaseConstantBuffer();
}

bool FRenderer::HandleHotReloadShader()
{
    if (ShaderManager->HandleHotReloadShader())
    {
        StaticMeshRenderPass->ReloadShader();
        GizmoRenderPass->ReloadShader();
        BillboardRenderPass->ReloadShader();
        FogRenderPass->ReloadShader();
        LineRenderPass->ReloadShader();
        UE_LOG(LogLevel::Display, "[Shader Hot Reload] Succeeded Shader Hot Reload");
        return true;
    }
    return false;
}

void FRenderer::PrepareRender()
{
    StaticMeshRenderPass->PrepareRender();
    GizmoRenderPass->PrepareRender();
    BillboardRenderPass->PrepareRender();
    UpdateLightBufferPass->PrepareRender();
    FogRenderPass->PrepareRender();
}

void FRenderer::ClearRenderArr()
{
    StaticMeshRenderPass->ClearRenderArr();
    BillboardRenderPass->ClearRenderArr();
    GizmoRenderPass->ClearRenderArr();
    UpdateLightBufferPass->ClearRenderArr();
    FogRenderPass->ClearRenderArr();
}

void FRenderer::Render(const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport()); // 지금부터 그림 글리 화면 영역 어디인지 알려줘

    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode()); // 뷰포트 설정에 맞는 그리기 방식 그래픽 카드에 설정

    //ChangeViewMode(ActiveViewport->GetViewMode());

    UpdateLightBufferPass->Render(ActiveViewport);
    StaticMeshRenderPass->Render(ActiveViewport);
    BillboardRenderPass->Render(ActiveViewport);
    

    if (IsSceneDepth)
    {
        DepthBufferDebugPass->RenderDepthBuffer(ActiveViewport);
    }

    if (!IsSceneDepth)
    {
        DepthBufferDebugPass->UpdateDepthBufferSRV();
        
        FogRenderPass->RenderFog(ActiveViewport, DepthBufferDebugPass->GetDepthSRV());
    }
    LineRenderPass->Render(ActiveViewport);
    GizmoRenderPass->Render(ActiveViewport);

    EditorRenderPass->Render(ActiveViewport);

    ClearRenderArr();
}
