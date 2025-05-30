#pragma once
#include "GameFramework/Actor.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectTypes.h"


class UGizmoBaseComponent;
class UGizmoArrowComponent;
class USceneComponent;
class UPrimitiveComponent;
class FEditorViewportClient;
class UStaticMeshComponent;

class AEditorPlayer : public AActor
{
    DECLARE_CLASS(AEditorPlayer, AActor)

    AEditorPlayer() = default;

    virtual void Tick(float DeltaTime) override;

    void Input();
    bool PickGizmo(FVector& rayOrigin, FEditorViewportClient* InActiveViewport);
    void ProcessGizmoIntersection(UStaticMeshComponent* iter, const FVector& pickPosition, FEditorViewportClient* InActiveViewport, bool& isPickedGizmo);
    void PickActor(const FVector& pickPosition);
    void AddControlMode();
    void AddCoordiMode();

private:
    int RayIntersectsObject(const FVector& pickPosition, USceneComponent* obj, float& hitDistance, int& intersectCount);
    void ScreenToViewSpace(int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& rayOrigin);
    void PickedObjControl();
    void ControlRotation(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY);
    void ControlScale(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY);

    bool bLeftMouseDown = false;

    POINT m_LastMousePos;
    ControlMode cMode = CM_TRANSLATION;
    CoordiMode cdMode = CDM_WORLD;

public:
    void SetMode(ControlMode _Mode) { cMode = _Mode; }
    ControlMode GetControlMode() const { return cMode; }
    CoordiMode GetCoordiMode() const { return cdMode; }
};
