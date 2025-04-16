#include "EditorRenderPass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "UObject/UObjectIterator.h"

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
#include "Components/Light/SpotLightComponent.h"

#include "BaseGizmos/GizmoBaseComponent.h"
#include "Engine/EditorEngine.h"

#include "PropertyEditor/ShowFlags.h"

#include "UnrealEd/EditorViewportClient.h"

FEditorRenderPass::FEditorRenderPass()
    : VertexShader(nullptr)
    , PixelShader(nullptr)
    , InputLayout(nullptr)
    , Stride(0)
    , BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FEditorRenderPass::~FEditorRenderPass()
{
}

void FEditorRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShader();
    CreateBuffer();
}

void FEditorRenderPass::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC LayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Stride = sizeof(float) * 3 + sizeof(float) * 4;

    HRESULT hr;
    hr = ShaderManager->AddVertexShaderAndInputLayout(L"SpotLightArrowVertexShader", L"Shaders/SpotLightArrow.hlsl", "mainVS", LayoutDesc, ARRAYSIZE(LayoutDesc));

    hr = ShaderManager->AddPixelShader(L"SpotLightArrowPixelShader", L"Shaders/SpotLightArrow.hlsl", "mainPS");

    ReloadShader();

    InputLayout = ShaderManager->GetInputLayoutByKey(L"SpotLightArrowVertexShader");
}

void FEditorRenderPass::PrepareRender()
{
    SpotLightObjs.Empty();

    for (USpotLightComponent* iter : TObjectRange<USpotLightComponent>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            SpotLightObjs.Add(iter);
        }
    }

    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &SpotLightArrowVertexBuffer.VertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    BufferManager->BindConstantBuffer(TEXT("FSpotLightArrowConstantBuffer"), 0, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(TEXT("FSpotLightArrowConstantBuffer"), 0, EShaderStage::Pixel);
}

void FEditorRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    PrepareRender();

    for (USpotLightComponent* SpotLight : SpotLightObjs)
    {
        FMatrix Model = SpotLight->GetWorldMatrix();
        FMatrix View = Viewport->GetViewMatrix();
        FMatrix Projection = Viewport->GetProjectionMatrix();
        FLinearColor Color = (SpotLight->GetDiffuseColor());
        UpdateConstantBuffer(Model, View, Projection, Color);
        Graphics->DeviceContext->Draw(2, 0);
    }  
}

void FEditorRenderPass::ClearRenderArr()
{
}

void FEditorRenderPass::ReloadShader()
{
    VertexShader = ShaderManager->GetVertexShaderByKey(L"SpotLightArrowVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"SpotLightArrowPixelShader");
}



void FEditorRenderPass::CreateBuffer()
{
    struct FSpotLightArrowVertex
    {
        FVector Position;
        FVector4 Color;
    };

    TArray<FSpotLightArrowVertex> Vertices;
    Vertices.Add(
        {
            FVector(0.0f, 0.0f, 0.0f), // Vertex position
            FVector4(1.0f, 1.0f, 1.0f, 1.0f) // Vertex color
        }
    );

    Vertices.Add(
        {
            FVector(10.0f, 0.0f, 0.0f),
            FVector4(1.0f, 1.0f, 1.0f, 1.0f)
        }
    );

    FVertexInfo VertexInfo;

    BufferManager->CreateVertexBuffer(L"SpotLightArrowVertexBuffer", Vertices, VertexInfo);

    SpotLightArrowVertexBuffer = VertexInfo;

    UINT SpotLightArrowConstantsBufferSize = sizeof(FSpotLightArrowConstantBuffer);
    BufferManager->CreateBufferGeneric<FSpotLightArrowConstantBuffer>("FSpotLightArrowConstantBuffer", nullptr, SpotLightArrowConstantsBufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

}

void FEditorRenderPass::UpdateConstantBuffer(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection, const FLinearColor color)
{
    FSpotLightArrowConstantBuffer Data(Model, View, Projection, color);
    
    BufferManager->UpdateConstantBuffer(TEXT("FSpotLightArrowConstantBuffer"), Data);
}
