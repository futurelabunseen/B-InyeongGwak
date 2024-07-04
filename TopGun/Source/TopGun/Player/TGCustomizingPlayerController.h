#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Utility/TGCustomizingComponent.h"
#include "GameInstance/TGGameInstance.h"
#include "TGCustomizingPlayerController.generated.h"

UENUM(BlueprintType)
enum class ECustomizingState : uint8
{
    Idle       UMETA(DisplayName = "Idle"),
    OnDragActor UMETA(DisplayName = "On Drag Actor"),
    OnSnappedActor UMETA(DisplayName = "On Snapped Actor"),
    OnRotateActor UMETA(DisplayName = "On Rotate Actor"),
    OnMouseMovement UMETA(DisplayName = "On Mouse Movement")
};

UCLASS()
class TOPGUN_API ATGCustomizingPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ATGCustomizingPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    // State Management
    void UpdateState();
    void EnterIdleState();
    void EnterDragState();
    void EnterSnappedState();
    void EnterRotateState();
    void EnterMouseMoveState();
    void ReturnToIdleState();

    // State-Specific Logic
    void HandleIdleState();
    void HandleDragState();
    void HandleSnappedState();
    void HandleRotateState();

    // Weapon Attach
    void UpdateWeaponActorPosition();
    void RemoveActorInDesiredPosition();
    void CheckSnappedCancellation();
    AActor* FindTargetActorUnderMouse() const;
    void TryFindRotatingTargetActor();

    // Input Related
    void OnClickRightMouse();
    void OnClickLeftMouse();
    void MouseLeftClickStarted();
    void StartMouseDrag();
    void MouseLeftClickEnded();
    void StopMouseDrag();
    void OnClickEscape();
    void OnRotateAction(const FInputActionValue& Value);
    void OnRotateCharacter(const FInputActionValue& Value);
    void AdjustCameraOffset(const FVector2D& MouseDelta);

    UFUNCTION(BlueprintCallable)
    void OnEnterAction();
    void OnRotateActor();

public:
    // UI
    UFUNCTION(BlueprintCallable)
    void AddWeaponButtonToPanel(class UScrollBox* TargetPanel);
    UFUNCTION(BlueprintCallable)
    void AddModuleButtonToPanel(class UScrollBox* TargetPanel);
    UFUNCTION(BlueprintCallable)
    void AddArmourButtonToPanel(UScrollBox* TargetPanel);
    UFUNCTION(BlueprintCallable)
    void OnWeaponSelected(FName WeaponID);
    UFUNCTION(BlueprintCallable)
    void OnArmourSelected(FName WeaponID);
    UFUNCTION(BlueprintCallable)
    void OnModuleSelected(FName WeaponID);

    // Variables
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputAction* LClickAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputAction* RClickAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputAction* EscapeAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputAction* RotateAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputAction* EnterAction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
    ECustomizingState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMultiplier")
    float RotationSpeed = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMultiplier")
    float MouseSensitivity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
    FName TargetLevelName;

    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* SpringArm;
    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CustomizingCamera;
    UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Camera")
    float MaxOffset = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Camera")
    float MinOffset = -300.0f;

private:
    bool bIsDragging;
    FVector2D MouseStartLocation;
    UTGCustomizingComponent* MyCustomizingComponent;
};
