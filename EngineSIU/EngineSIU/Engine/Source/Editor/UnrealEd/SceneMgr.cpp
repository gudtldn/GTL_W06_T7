#include "UnrealEd/SceneMgr.h"
#include <fstream>

#include "CoreMiscDefines.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CubeComp.h"
#include "Components/Light/LightComponent.h"
#include "Components/SkySphereComponent.h"
#include "Components/SphereComp.h"
#include "Components/BillboardComponent.h"
#include "Engine/Engine.h"
#include "JSON/json.hpp"
#include "UObject/Casts.h"
#include "UObject/Object.h"
#include "UObject/ObjectFactory.h"
#include "World/World.h"

using json = nlohmann::json;

SceneData FSceneMgr::ParseSceneData(const FString& jsonStr)
{
    SceneData sceneData;

    try {
        json j = json::parse(*jsonStr);

        // 버전과 NextUUID 읽기
        sceneData.Version = j["Version"].get<int>();
        sceneData.NextUUID = j["NextUUID"].get<int>();

        // Primitives 처리 (C++14 스타일)
        auto primitives = j["Primitives"];
        for (auto it = primitives.begin(); it != primitives.end(); ++it) {
            int id = std::stoi(it.key());  // Key는 문자열, 숫자로 변환
            const json& value = it.value();
            UObject* obj = nullptr;
            if (value.contains("Type"))
            {
                const FString TypeName = value["Type"].get<std::string>();
                if (TypeName == USphereComp::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<USphereComp>(GEngine->ActiveWorld);
                }
                else if (TypeName == UCubeComp::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UCubeComp>(GEngine->ActiveWorld);
                }
                else if (TypeName == UGizmoArrowComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UGizmoArrowComponent>(GEngine->ActiveWorld);
                }
                else if (TypeName == UBillboardComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<UBillboardComponent>(GEngine->ActiveWorld);
                }
                else if (TypeName == ULightComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<ULightComponent>(GEngine->ActiveWorld);
                }
                else if (TypeName == USkySphereComponent::StaticClass()->GetName())
                {
                    obj = FObjectFactory::ConstructObject<USkySphereComponent>(GEngine->ActiveWorld);
                }
            }

            USceneComponent* sceneComp = Cast<USceneComponent>(obj);
            //Todo : 여기다가 Obj Maeh저장후 일기
            //if (value.contains("ObjStaticMeshAsset"))
            if (value.contains("Location")) sceneComp->SetRelativeLocation(FVector(value["Location"].get<std::vector<float>>()[0],
                value["Location"].get<std::vector<float>>()[1],
                value["Location"].get<std::vector<float>>()[2]));
            if (value.contains("Rotation")) sceneComp->SetRelativeRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
                                                                                   value["Rotation"].get<std::vector<float>>()[1],
                                                                                   value["Rotation"].get<std::vector<float>>()[2]));
            if (value.contains("Scale")) sceneComp->SetRelativeScale3D(FVector(value["Scale"].get<std::vector<float>>()[0],
                value["Scale"].get<std::vector<float>>()[1],
                value["Scale"].get<std::vector<float>>()[2]));
            if (value.contains("Type")) {
                UPrimitiveComponent* primitiveComp = Cast<UPrimitiveComponent>(sceneComp);
                if (primitiveComp) {
                    primitiveComp->SetType(value["Type"].get<std::string>());
                }
                else {
                    std::string name = value["Type"].get<std::string>();
                    sceneComp->NamePrivate = name.c_str();
                }
            }
            sceneData.Primitives[id] = sceneComp;
        }

        // auto perspectiveCamera = j["PerspectiveCamera"];
        // for (auto it = perspectiveCamera.begin(); it != perspectiveCamera.end(); ++it) {
        //     int id = std::stoi(it.key());  // Key는 문자열, 숫자로 변환
        //     const json& value = it.value();
        //     UObject* obj = FObjectFactory::ConstructObject<UCameraComponent>(nullptr);
        //     UCameraComponent* camera = Cast<UCameraComponent>(obj);
        //     if (value.contains("Location")) camera->SetRelativeLocation(FVector(value["Location"].get<std::vector<float>>()[0],
        //             value["Location"].get<std::vector<float>>()[1],
        //             value["Location"].get<std::vector<float>>()[2]));
        //     if (value.contains("Rotation")) camera->SetRelativeRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
        //                                                                         value["Rotation"].get<std::vector<float>>()[1],
        //                                                                         value["Rotation"].get<std::vector<float>>()[2]));
        //     if (value.contains("Rotation")) camera->SetRelativeRotation(FVector(value["Rotation"].get<std::vector<float>>()[0],
        //                                                                         value["Rotation"].get<std::vector<float>>()[1],
        //                                                                         value["Rotation"].get<std::vector<float>>()[2]));
        //     if (value.contains("FOV")) camera->SetFOV(value["FOV"].get<float>());
        //     if (value.contains("NearClip")) camera->SetNearClip(value["NearClip"].get<float>());
        //     if (value.contains("FarClip")) camera->SetNearClip(value["FarClip"].get<float>());
        //     
        //     
        //     sceneData.Cameras[id] = camera;
        // }
    }
    catch (const std::exception& e) {
        FString errorMessage = "Error parsing JSON: ";
        errorMessage += e.what();

        UE_LOG(LogLevel::Error, "No Json file");
    }

    return sceneData;
}

FString FSceneMgr::LoadSceneFromFile(const FString& filename)
{
    std::ifstream inFile(*filename);
    if (!inFile) {
        UE_LOG(LogLevel::Error, "Failed to open file for reading: %s", *filename);
        return FString();
    }

    json j;
    try {
        inFile >> j; // JSON 파일 읽기
    }
    catch (const std::exception& e) {
        UE_LOG(LogLevel::Error, "Error parsing JSON: %s", e.what());
        return FString();
    }

    inFile.close();

    return j.dump(4);
}

std::string FSceneMgr::SerializeSceneData(const SceneData& sceneData)
{
    json j;

    // Version과 NextUUID 저장
    j["Version"] = sceneData.Version;
    j["NextUUID"] = sceneData.NextUUID;

    // Primitives 처리 (C++17 스타일)
    for (const auto& [Id, Obj] : sceneData.Primitives)
    {
        USceneComponent* primitive = static_cast<USceneComponent*>(Obj);
        std::vector<float> Location = { primitive->GetWorldLocation().X,primitive->GetWorldLocation().Y,primitive->GetWorldLocation().Z };
        std::vector<float> Rotation = { primitive->GetWorldRotation().Roll,primitive->GetWorldRotation().Pitch,primitive->GetWorldRotation().Yaw };
        std::vector<float> Scale = { primitive->GetWorldScale3D().X,primitive->GetWorldScale3D().Y,primitive->GetWorldScale3D().Z };

        std::string primitiveName = *primitive->GetName();
        size_t pos = primitiveName.rfind('_');
        if (pos != INDEX_NONE) {
            primitiveName = primitiveName.substr(0, pos);
        }
        j["Primitives"][std::to_string(Id)] = {
            {"Location", Location},
            {"Rotation", Rotation},
            {"Scale", Scale},
            {"Type",primitiveName}
        };
    }

    // for (const auto& [id, camera] : sceneData.Cameras)
    // {
    //     UCameraComponent* cameraComponent = static_cast<UCameraComponent*>(camera);
    //     TArray<float> Location = { cameraComponent->GetWorldLocation().X, cameraComponent->GetWorldLocation().Y, cameraComponent->GetWorldLocation().Z };
    //     TArray<float> Rotation = { 0.0f, cameraComponent->GetWorldRotation().Pitch, cameraComponent->GetWorldRotation().Yaw };
    //     float FOV = cameraComponent->GetFOV();
    //     float nearClip = cameraComponent->GetNearClip();
    //     float farClip = cameraComponent->GetFarClip();
    //
    //     //
    //     j["PerspectiveCamera"][std::to_string(id)] = {
    //         {"Location", Location},
    //         {"Rotation", Rotation},
    //         {"FOV", FOV},
    //         {"NearClip", nearClip},
    //         {"FarClip", farClip}
    //     };
    // }

    return j.dump(4); // 4는 들여쓰기 수준
}

bool FSceneMgr::SaveSceneToFile(const FString& filename, const SceneData& sceneData)
{
    std::ofstream outFile(*filename);
    if (!outFile) {
        FString errorMessage = "Failed to open file for writing: ";
        MessageBoxA(nullptr, *errorMessage, "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    std::string jsonData = SerializeSceneData(sceneData);
    outFile << jsonData;
    outFile.close();

    return true;
}

