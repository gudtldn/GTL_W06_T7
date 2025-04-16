#include "DXDShaderManager.h"
#include <d3dcompiler.h>
#include <fstream>

#include "UserInterface/Console.h"

using namespace NS_ShaderMetadata;


class FShaderIncludeHandler : public ID3DInclude
{
public:
    FShaderIncludeHandler() = default;
    virtual ~FShaderIncludeHandler() = default;

    [[nodiscard]] IncludesMetadata GetIncludePaths() const
    {
        return IncludePaths;
    }

protected:
    virtual HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) noexcept override
    {
        const fs::path AbsolutePath = absolute(fs::path("Shaders") / pFileName);
        IncludePaths.Add(MakePair(AbsolutePath, fs::last_write_time(AbsolutePath)));

        // 파일 열기
        std::ifstream File(AbsolutePath, std::ios::binary);
        if (!File.is_open())
        {
            return E_FAIL;
        }

        File.seekg(0, std::ios::end);
        const int64 Size = File.tellg();
        File.seekg(0, std::ios::beg);

        char* Data = new char[Size];
        File.read(Data, Size);
        File.close();

        *ppData = Data;
        *pBytes = static_cast<UINT>(Size);

        return S_OK;
    }

    virtual HRESULT Close(LPCVOID pData) noexcept override
    {
        delete[] static_cast<const char*>(pData);
        return S_OK;
    }

private:
    IncludesMetadata IncludePaths;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};


FDXDShaderManager::FDXDShaderManager(ID3D11Device* Device)
    : DXDDevice(Device)
{
    VertexShaders.Empty();
    PixelShaders.Empty();
}

void FDXDShaderManager::ReleaseAllShader()
{
    for (auto& [Key, Shader] : VertexShaders)
    {
        if (Shader)
        {
            Shader->Release();
            Shader = nullptr;
        }
    }
    VertexShaders.Empty();

    for (auto& [Key, Shader] : PixelShaders)
    {
        if (Shader)
        {
            Shader->Release();
            Shader = nullptr;
        }
    }
    PixelShaders.Empty();

}

HRESULT FDXDShaderManager::AddPixelShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    HRESULT hr = S_OK;

    if (DXDDevice == nullptr)
        return S_FALSE;

    ID3DBlob* PsBlob = nullptr;
    FShaderIncludeHandler IncludesHandler;
    hr = D3DCompileFromFile(FileName.c_str(), nullptr, &IncludesHandler, EntryPoint.c_str(), "ps_5_0", shaderFlags, 0, &PsBlob, nullptr);
    if (FAILED(hr))
        return hr;

    ID3D11PixelShader* NewPixelShader;
    hr = DXDDevice->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &NewPixelShader);
    if (PsBlob)
    {
        PsBlob->Release();
    }
    if (FAILED(hr))
        return hr;

    PixelShaders[Key] = NewPixelShader;
    PixelShaders[Key].SetFileMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths()));

    return S_OK;
}

HRESULT FDXDShaderManager::AddPixelShader(
    const std::wstring& Key,
    const std::wstring& FileName,
    const std::string& EntryPoint,
    const D3D_SHADER_MACRO* Defines)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    if (DXDDevice == nullptr)
        return S_FALSE;

    ID3DBlob* PsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(
        FileName.c_str(),
        Defines, // << 매크로 정의 들어가는 핵심 부분
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPoint.c_str(),
        "ps_5_0",
        shaderFlags,
        0,
        &PsBlob,
        &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            std::string errMsg((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize());
            std::cerr << "Shader compile error: " << errMsg << std::endl;
            errorBlob->Release();
        }
        return hr;
    }
        

    ID3D11PixelShader* NewPixelShader;
    hr = DXDDevice->CreatePixelShader(PsBlob->GetBufferPointer(), PsBlob->GetBufferSize(), nullptr, &NewPixelShader);

    if (PsBlob)
        PsBlob->Release();

    if (FAILED(hr))
        return hr;

    PixelShaders[Key] = NewPixelShader;

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShader(const std::wstring& Key, const std::wstring& FileName)
{
    return E_NOTIMPL;
}

HRESULT FDXDShaderManager::AddVertexShader(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    FShaderIncludeHandler IncludesHandler;
    hr = D3DCompileFromFile(FileName.c_str(), nullptr, &IncludesHandler, EntryPoint.c_str(), "vs_5_0", 0, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;
    VertexShaders[Key].SetFileMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths()));

    VertexShaderCSO->Release();

    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShader(
    const std::wstring& Key,
    const std::wstring& FileName,
    const std::string& EntryPoint,
    const D3D_SHADER_MACRO* Defines)
{
    if (DXDDevice == nullptr)
        return S_FALSE;

    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        FileName.c_str(),
        Defines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPoint.c_str(),
        "vs_5_0",
        shaderFlags,
        0,
        &vsBlob,
        &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        vsBlob->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;
    vsBlob->Release();

    return S_OK;
}

HRESULT FDXDShaderManager::AddInputLayout(const std::wstring& Key, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize)
{
    return S_OK;
}

HRESULT FDXDShaderManager::AddVertexShaderAndInputLayout(const std::wstring& Key, const std::wstring& FileName, const std::string& EntryPoint, const D3D11_INPUT_ELEMENT_DESC* Layout, uint32_t LayoutSize)
{
    UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    if (DXDDevice == nullptr)
        return S_FALSE;

    HRESULT hr = S_OK;

    ID3DBlob* VertexShaderCSO = nullptr;
    ID3DBlob* ErrorBlob = nullptr;

    FShaderIncludeHandler IncludesHandler;
    hr = D3DCompileFromFile(FileName.c_str(), nullptr, &IncludesHandler, EntryPoint.c_str(), "vs_5_0", shaderFlags, 0, &VertexShaderCSO, &ErrorBlob);
    if (FAILED(hr))
    {
        if (ErrorBlob) {
            OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
            ErrorBlob->Release();
        }
        return hr;
    }

    ID3D11VertexShader* NewVertexShader;
    hr = DXDDevice->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &NewVertexShader);
    if (FAILED(hr))
    {
        return hr;
    }

    ID3D11InputLayout* NewInputLayout;
    hr = DXDDevice->CreateInputLayout(Layout, LayoutSize, VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &NewInputLayout);
    if (FAILED(hr))
    {
        VertexShaderCSO->Release();
        return hr;
    }

    VertexShaders[Key] = NewVertexShader;
    VertexShaders[Key].SetFileMetadata(std::make_unique<FShaderFileMetadata>(EntryPoint, FileName, IncludesHandler.GetIncludePaths()));
    InputLayouts[Key] = NewInputLayout;

    VertexShaderCSO->Release();

    return S_OK;
}

ID3D11InputLayout* FDXDShaderManager::GetInputLayoutByKey(const std::wstring& Key) const
{
    if (InputLayouts.Contains(Key))
    {
        return *InputLayouts.Find(Key);
    }
    return nullptr;
}

ID3D11VertexShader* FDXDShaderManager::GetVertexShaderByKey(const std::wstring& Key) const
{
    if (VertexShaders.Contains(Key))
    {
        return *VertexShaders.Find(Key);
    }
    return nullptr;
}

ID3D11PixelShader* FDXDShaderManager::GetPixelShaderByKey(const std::wstring& Key) const
{
    if (PixelShaders.Contains(Key))
    {
        return *PixelShaders.Find(Key);
    }
    return nullptr;
}

FShaderPipeline FDXDShaderManager::GetShaderPipelineByLightingModel(ELightingModel model) const
{
    static const std::wstring keys[] = {
        L"Gouraud", L"Lambert", L"BlinnPhong", L"Unlit"
    };

    const std::wstring& psKey = keys[static_cast<int>(model)];
    const std::wstring& vsKey = (model == ELightingModel::Gouraud)
        ? psKey
        : L"StaticMeshVertexShader";

    FShaderPipeline pipeline;
    pipeline.VertexShader = GetVertexShaderByKey(vsKey);
    pipeline.PixelShader = GetPixelShaderByKey(psKey);

    return pipeline;
}

void FDXDShaderManager::RegisterShaderVariants()
{
    const std::wstring vsPath = L"Shaders/StaticMeshVertexShader.hlsl";
    const std::wstring psPath = L"Shaders/StaticMeshPixelShader.hlsl";
    const std::string vsEntry = "mainVS";
    const std::string psEntry = "mainPS";

    struct Variant
    {
        std::wstring Key;
        std::string LightingModelValue;
    };

    std::vector<Variant> variants = {
        { L"Gouraud",    "1" },
        { L"Lambert",    "2" },
        { L"BlinnPhong", "3" },
        { L"Unlit",      "4" },
    };
    for (const auto& variant : variants)
    {
        // Gouraud일 경우에만 Vertex Shader 따로 생성
        if (variant.Key == variants[0].Key)
        {
            if (!VertexShaders.Contains(variant.Key))
            {
                D3D_SHADER_MACRO vsDefines[] = {
                    { "VERTEX_SHADER", "1" },
                    { "LIGHTING_MODEL", variant.LightingModelValue.c_str() },
                    { nullptr, nullptr }
                };

                HRESULT hr = AddVertexShader(variant.Key, vsPath, vsEntry, vsDefines);
                if (FAILED(hr))
                {
                    OutputDebugStringA(("Failed to compile vertex shader: " + std::string(variant.Key.begin(), variant.Key.end()) + "\n").c_str());
                }
            }
        }
        
        if (!PixelShaders.Contains(variant.Key))
        {
            D3D_SHADER_MACRO psDefines[] = {
                { "PIXEL_SHADER", "1" },
                { "LIGHTING_MODEL", variant.LightingModelValue.c_str() },
                { nullptr, nullptr }
            };

            HRESULT hr = AddPixelShader(variant.Key, psPath, psEntry, psDefines);
            if (FAILED(hr))
            {
                OutputDebugStringA(("Failed to compile pixel shader: " + std::string(variant.Key.begin(), variant.Key.end()) + "\n").c_str());
            }
        }
    }
}

bool FDXDShaderManager::HandleHotReloadShader()
{
    bool bIsHotReloadShader = false;
    for (auto& Vs : VertexShaders)
    {
        FShaderFileMetadata& Data = Vs.Value.GetShaderMetadata();
        if (Vs.Value.GetShaderMetadata().IsOutdatedAndUpdateLastTime())
        {
            ID3DBlob* VertexShaderCSO = nullptr;
            ID3DBlob* ErrorBlob = nullptr;

            // 셰이더 컴파일
            FShaderIncludeHandler IncludesHandler;
            HRESULT Hr = D3DCompileFromFile(
                Data.FileMetadata.Key.c_str(), nullptr, &IncludesHandler, *Data.EntryPoint,
                "vs_5_0", 0, 0, &VertexShaderCSO, &ErrorBlob
            );

            // 셰이더 컴파일 실패시
            if (FAILED(Hr))
            {
                if (ErrorBlob)
                {
                    UE_LOG(LogLevel::Error, "[Shader Hot Reload] VertexShader Compile Failed %s", static_cast<char*>(ErrorBlob->GetBufferPointer()));
                    ErrorBlob->Release();
                }
                continue;
            }

            ID3D11VertexShader* NewVertexShader;
            Hr = DXDDevice->CreateVertexShader(
                VertexShaderCSO->GetBufferPointer(),
                VertexShaderCSO->GetBufferSize(),
                nullptr, &NewVertexShader
            );

            // VS 만들기 실패시
            if (FAILED(Hr))
            {
                UE_LOG(LogLevel::Error, "[Shader Hot Reload] Failed CreateVertexShader");
                VertexShaderCSO->Release();
                continue;
            }

            // 기존 셰이더 제거
            Vs.Value->Release();

            // 새로운 셰이더 할당
            Vs.Value = NewVertexShader;
            Data.IncludePaths = IncludesHandler.GetIncludePaths();

            VertexShaderCSO->Release();
            bIsHotReloadShader = true;
        }
    }

    for (auto& Ps : PixelShaders)
    {
        FShaderFileMetadata& Data = Ps.Value.GetShaderMetadata();
        if (Ps.Value.GetShaderMetadata().IsOutdatedAndUpdateLastTime())
        {
            ID3DBlob* PixelShaderCSO = nullptr;
            ID3DBlob* ErrorBlob = nullptr;

            // 셰이더 컴파일
            FShaderIncludeHandler IncludesHandler;
            HRESULT Hr = D3DCompileFromFile(
                Data.FileMetadata.Key.c_str(), nullptr, &IncludesHandler, *Data.EntryPoint,
                "ps_5_0", 0, 0, &PixelShaderCSO, &ErrorBlob
            );

            // 셰이더 컴파일 실패시
            if (FAILED(Hr))
            {
                if (ErrorBlob)
                {
                    UE_LOG(LogLevel::Error, "[Shader Hot Reload] PixelShader Compile Failed %s", static_cast<char*>(ErrorBlob->GetBufferPointer()));
                    ErrorBlob->Release();
                }
                continue;
            }

            ID3D11PixelShader* NewPixelShader;
            Hr = DXDDevice->CreatePixelShader(
                PixelShaderCSO->GetBufferPointer(),
                PixelShaderCSO->GetBufferSize(),
                nullptr, &NewPixelShader
            );

            // PS 만들기 실패시
            if (FAILED(Hr))
            {
                UE_LOG(LogLevel::Error, "[Shader Hot Reload] Failed CreatePixelShader");
                PixelShaderCSO->Release();
                continue;
            }

            // 기존 셰이더 제거
            Ps.Value->Release();

            // 새로운 셰이더 할당
            Ps.Value = NewPixelShader;
            Data.IncludePaths = IncludesHandler.GetIncludePaths();

            PixelShaderCSO->Release();
            bIsHotReloadShader = true;
        }
    }
    return bIsHotReloadShader;
}
