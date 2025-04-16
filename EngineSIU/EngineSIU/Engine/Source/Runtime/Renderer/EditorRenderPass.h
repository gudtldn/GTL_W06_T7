#pragma once
#include "IRenderPass.h"

#include "EngineBaseTypes.h"
#include "Container/Array.h"

#include "Define.h"

class UStaticMeshComponent;

class USpotLightComponent;

struct FSpotLightArrowConstantBuffer
{
    FMatrix Model;
    FMatrix View;
    FMatrix Projection;
    FLinearColor Color;
};

class FEditorRenderPass : public IRenderPass
{
public:
    FEditorRenderPass();

    ~FEditorRenderPass();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    void CreateShader();
    virtual void PrepareRender() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

    void ReloadShader();

    void CreateBuffer();

    void UpdateConstantBuffer(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection, const FLinearColor color);

private:
    TArray<USpotLightComponent*> SpotLightObjs;

    ID3D11VertexShader* VertexShader;

    ID3D11PixelShader* PixelShader;

    ID3D11InputLayout* InputLayout;

    uint32 Stride;

    FDXDBufferManager* BufferManager;

    FGraphicsDevice* Graphics;

    FDXDShaderManager* ShaderManager;

    FVertexInfo SpotLightArrowVertexBuffer;
};

