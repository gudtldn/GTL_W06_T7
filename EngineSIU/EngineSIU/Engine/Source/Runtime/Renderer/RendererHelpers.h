#pragma once
#include "Launch/EngineLoop.h"

namespace RendererHelpers {
    
    inline FMatrix CalculateMVP(const FMatrix& Model, const FMatrix& View, const FMatrix& Projection) {
        return Model * View * Projection;
    }

    inline FMatrix CalculateNormalMatrix(const FMatrix& Model) {
        return FMatrix::Transpose(FMatrix::Inverse(Model));
    }
}

namespace MaterialUtils {
    inline void UpdateMaterial(FDXDBufferManager* BufferManager, FGraphicsDevice* Graphics, const FObjMaterialInfo& MaterialInfo) {
        FMaterialConstants data;
        data.DiffuseColor = MaterialInfo.Diffuse;
        data.TransparencyScalar = MaterialInfo.TransparencyScalar;
        data.AmbientColor = MaterialInfo.Ambient;
        data.DensityScalar = MaterialInfo.DensityScalar;
        data.SpecularColor = MaterialInfo.Specular;
        data.SpecularScalar = MaterialInfo.SpecularScalar;
        data.EmmisiveColor = MaterialInfo.Emissive;
        data.bUseBumpMap = MaterialInfo.bHasBumpTexture ? 1 : 0;

        
        if (MaterialInfo.bHasDiffuseTexture) {
            std::shared_ptr<FTexture> texture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.DiffuseTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
        else {
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            ID3D11SamplerState* nullSampler[1] = { nullptr };
            Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
        }

        // Normal Map Texture 바인딩
        if (MaterialInfo.bHasBumpTexture)
        {
            std::shared_ptr<FTexture> BumpTexture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.BumpTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(1, 1, &BumpTexture->TextureSRV); // t1 슬롯
            Graphics->DeviceContext->PSSetSamplers(1, 1, &BumpTexture->SamplerState); // s1 슬롯
        }
        else
        {
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            ID3D11SamplerState* nullSampler[1] = { nullptr };
            Graphics->DeviceContext->PSSetShaderResources(1, 1, nullSRV);
            Graphics->DeviceContext->PSSetSamplers(1, 1, nullSampler);
        }

        BufferManager->UpdateConstantBuffer(TEXT("FMaterialConstants"), data);

    }
}
