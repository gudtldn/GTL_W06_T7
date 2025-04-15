#pragma once
#include <sstream>
#include "Define.h"
#include "Container/Map.h"

class SSplitterH;
class SSplitterV;
class UWorld;
class FEditorViewportClient;


class SLevelEditor
{
public:
    SLevelEditor();

    void Initialize();
    void Tick(float DeltaTime);
    void Release();

    void SelectViewport(FVector2D InPoint);

    void ResizeLevelEditor();
    void ResizeViewports();
    void SetEnableMultiViewport(bool bIsEnable);
    bool IsMultiViewport() const;

private:
    SSplitterH* HSplitter;
    SSplitterV* VSplitter;
    std::shared_ptr<FEditorViewportClient> ViewportClients[4];
    std::shared_ptr<FEditorViewportClient> ActiveViewportClient;

    /** 우클릭 시 캡처된 마우스 커서의 초기 위치 (스크린 좌표계) */
    FVector2D MousePinPosition;

    /** 우클릭이 눌려있는지 여부 */
    bool bIsPressedMouseRightButton = false;

    bool bMultiViewportMode;

    float EditorWidth;
    float EditorHeight;

public:
    std::shared_ptr<FEditorViewportClient>* GetViewports() { return ViewportClients; }
    std::shared_ptr<FEditorViewportClient> GetActiveViewportClient() const
    {
        return ActiveViewportClient;
    }
    void SetViewportClient(const std::shared_ptr<FEditorViewportClient>& viewportClient)
    {
        ActiveViewportClient = viewportClient;
    }
    void SetViewportClient(int index)
    {
        ActiveViewportClient = ViewportClients[index];
    }

    //Save And Load
private:
    const FString IniFilePath = "editor.ini";

public:
    void LoadConfig();
    void SaveConfig();

private:
    TMap<FString, FString> ReadIniFile(const FString& filePath);
    void WriteIniFile(const FString& filePath, const TMap<FString, FString>& config);

    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue) {
        if (const FString* Value = config.Find(key))
        {
            std::istringstream iss(**Value);
            T value;
            if (iss >> value)
            {
                return value;
            }
        }
        return defaultValue;
    }
};
