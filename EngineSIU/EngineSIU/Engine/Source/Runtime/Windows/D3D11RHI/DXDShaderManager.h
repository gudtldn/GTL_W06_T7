#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>
#include <filesystem>
#include <memory>

#include "Container/Array.h"
#include "Container/Map.h"


namespace NS_ShaderMetadata
{
namespace fs = std::filesystem;
using IncludesMetadata = TArray<TPair<fs::path, fs::file_time_type>>;


template <typename T>
concept ShaderConcept =
    std::is_same_v<T, ID3D11ComputeShader>
    || std::is_same_v<T, ID3D11VertexShader>
    || std::is_same_v<T, ID3D11PixelShader>
    || std::is_same_v<T, ID3D11GeometryShader>
    || std::is_same_v<T, ID3D11HullShader>
    || std::is_same_v<T, ID3D11DomainShader>;

struct FShaderFileMetadata
{
    FShaderFileMetadata(const FString& InEntryPoint, fs::path InPath, IncludesMetadata InIncludePaths)
        : EntryPoint(InEntryPoint)
        , FileMetadata(std::move(InPath), last_write_time(InPath))
        , IncludePaths(std::move(InIncludePaths))
    {
    }

    /**
     * .hlsl파일이 변경되었는지 확인하고, 마지막으로 수정된 시간을 갱신합니다.
     * @param bIsCheckIncludes .hlsl의 Include 목록까지 검사할지 여부
     * @return 파일이 변경되었는지 여부
     */
    bool IsOutdatedAndUpdateLastTime(bool bIsCheckIncludes = true)
    {
        const fs::file_time_type NewLastWriteTime = last_write_time(FileMetadata.Key);
        if (FileMetadata.Value != NewLastWriteTime)
        {
            FileMetadata.Value = NewLastWriteTime;
            return true;
        }

        if (bIsCheckIncludes)
        {
            for (auto& Include : IncludePaths)
            {
                const fs::file_time_type NewIncludeLastWriteTime = last_write_time(Include.Key);
                if (Include.Value != NewIncludeLastWriteTime)
                {
                    Include.Value = NewIncludeLastWriteTime;
                    return true;
                }
            }
        }
        return false;
    }

    // 셰이더 호출 시작지점
    FString EntryPoint;

    // 파일 경로, 마지막으로 수정된 시간
    TPair<fs::path, fs::file_time_type> FileMetadata;

    // .hlsl에 include된 파일 경로 목록
    IncludesMetadata IncludePaths;
};

/**
 * Shader Hot Reload를 위한, 파일 정보를 담고 있는 구조체입니다.
 */
template <ShaderConcept ShaderType>
class TShaderMetadataPtr
{
public:
    TShaderMetadataPtr()
        : ShaderPtr(nullptr)
    {
    }

    TShaderMetadataPtr(nullptr_t)
        : ShaderPtr(nullptr)
    {
    }

    TShaderMetadataPtr(ShaderType* RawShaderPtr)
        : ShaderPtr(RawShaderPtr)
    {
    }

    TShaderMetadataPtr& operator=(ShaderType* RawShaderPtr)
    {
        ShaderPtr = RawShaderPtr;
        return *this;
    }

    TShaderMetadataPtr& operator=(nullptr_t)
    {
        ShaderPtr = nullptr;
        return *this;
    }

    FORCEINLINE ShaderType* Get() const { return ShaderPtr; }

    FORCEINLINE ShaderType* operator->() { return Get(); }
    FORCEINLINE ShaderType& operator*() { return *Get(); }
    FORCEINLINE operator ShaderType*() const { return Get(); }
    FORCEINLINE operator bool() const { return Get(); }

public:
    void SetFileMetadata(std::unique_ptr<FShaderFileMetadata> InMetadata)
    {
        Metadata = std::move(InMetadata);
    }

    FShaderFileMetadata& GetShaderMetadata() const
    {
        return *Metadata;
    }

private:
    ShaderType* ShaderPtr;
    std::unique_ptr<FShaderFileMetadata> Metadata;
};
}

struct FVertexShaderData
{
    ID3DBlob* VertexShaderCSO;
    ID3D11VertexShader* VertexShader;
};

class FDXDShaderManager
{
public:
	FDXDShaderManager() = default;
	FDXDShaderManager(ID3D11Device* Device);

	void ReleaseAllShader();

private:
	ID3D11Device* DXDDevice;

public:
	HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName);
	HRESULT AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);
	HRESULT AddInputLayout(const std::wstring& Key, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize);

	
	HRESULT AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize);

	HRESULT AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint);

	ID3D11InputLayout* GetInputLayoutByKey(const std::wstring& Key) const;
	ID3D11VertexShader* GetVertexShaderByKey(const std::wstring& Key) const;
	ID3D11PixelShader* GetPixelShaderByKey(const std::wstring& Key) const;

    /** 셰이더를 HotReload 합니다. */
    bool HandleHotReloadShader();

private:
	TMap<std::wstring, ID3D11InputLayout*> InputLayouts;
	// TMap<std::wstring, ID3D11VertexShader*> VertexShaders;
	// TMap<std::wstring, ID3D11PixelShader*> PixelShaders;
	TMap<std::wstring, NS_ShaderMetadata::TShaderMetadataPtr<ID3D11VertexShader>> VertexShaders;
	TMap<std::wstring, NS_ShaderMetadata::TShaderMetadataPtr<ID3D11PixelShader>> PixelShaders;
};

