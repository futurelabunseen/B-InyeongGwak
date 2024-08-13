#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Utility/TGCustomizingComponent.h"
#include "GameInstance/TGGameInstance.h"
#include "TGCustomizingPlayerController.generated.h"

UENUM(BlueprintType)
enum class ECustomizingState : uint8
{
    Idle       UMETA(DisplayName = "Idle"),
    OnDragActor UMETA(DisplayName = "On Drag Actor"),
    OnSnappedActor UMETA(DisplayName = "On Snapped Actor"),
    OnSelectActor   UMETA(DisplayName = "On Select Actor"),
    OnRotateEquip UMETA(DisplayName = "On Rotate Actor"),
    OnBindKey UMETA(DisplayName = "On Bind Key"),
};


UCLASS()
class TOPGUN_API ATGCustomizingPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ATGCustomizingPlayerController();
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotationUpdated, FText, NewNotation);
    
protected:
    virtual void BeginPlay() override;
    void OnPressDeleteEquipmentAction();
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    // State Management
    void UpdateState();
    void EnterIdleState();
    void EnterDragState();
    void EnterSnappedState();
    void EnterRotateState();
    void ReturnToIdleState();
    void EnterSelectActorState();
    void ReturnToSelectActorState();
    void HandleSelectActorState();
    void EnterBindKeyState();

    // State-Specific Logic
    void HandleIdleState();
    void HandleDragState();
    void HandleSnappedState();
    void HandleRotateState();
    void HandleBindKeyState();


    // Equip Attach
    void UpdateEquipActorPosition();
    void CheckSnappedCancellation();
    AActor* FindTargetActorUnderMouse() const;
    void TryFindSelectActor();
    void ClearCurrentEquipment() const;

    // Input Related
    void OnClickRightMouse();
    void OnClickLeftMouse();
    void MouseLeftClickStarted();
    void StartMouseDrag();
    void MouseLeftClickEnded();
    void StopMouseDrag();
    void OnClickEscape();
    void OnRotateAction(const FInputActionValue& Value);
    void OnRotateCameraZoom(const FInputActionValue& Value);
    void OnRotateCharacter(const FInputActionValue& Value);

    //InputBinding
    bool IsInputKeyDown(const FKey& Key) const;
    FKey GetActionPressedKey(const UInputAction* Action) const;
    virtual void ProcessPlayerInput(float DeltaTime, bool bGamePaused) override;
    bool IsValidKeyForBinding(const FKey& Key) const;
    void HandleKeyBindingInput(const FKey& Key);

    UFUNCTION(BlueprintCallable)
    void OnEnterAction();
    void OnRotateEquipment();

public:
    // UI
    UFUNCTION(BlueprintCallable)
    void BindNotationUpdatedEvent(UTextBlock* NotationTextBlock);
    UFUNCTION(BlueprintCallable)
    void AddWeaponButtonToPanel(class UScrollBox* TargetPanel);
    UFUNCTION(BlueprintCallable)
    void AddModuleButtonToPanel(class UScrollBox* TargetPanel);
    UFUNCTION(BlueprintCallable)
    void AddArmourButtonToPanel(UScrollBox* TargetPanel);
    UFUNCTION(BlueprintCallable)
    void OnEquipSelect(FName WeaponID);
    UFUNCTION(BlueprintCallable)
    void OnModuleSelected(FName WeaponID);
    void SwitchToZoomedCamera(AActor* FocusActor);
    void ReturnToDefaultCamera();
    void UpdateZoomedCameraPosition();
    UFUNCTION(BlueprintCallable)
    void RegisterWeaponSelectButton(UUserWidget* TargetWidget);
    void UpdateNotationUI(const FString& NewText);
    void ClearNotationUI();
    UFUNCTION(BlueprintCallable)
    void RemoveActorInDesiredPosition();
    UFUNCTION(BlueprintCallable)
    void OnPressEnterRotateEquipment();
    UFUNCTION(BlueprintCallable)
    void OnPressKeyBindingEquipment();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWidget")
    TSubclassOf<UUserWidget> InventoryWidgetTemplate;
    UFUNCTION(BlueprintCallable)
    void OnPressReturnToIdleState();
    UPROPERTY()
    TSoftObjectPtr<UUserWidget> CurrentWeaponToolWidget;
    UPROPERTY()
    TSoftObjectPtr<UUserWidget> CurrentInventoryWidget;
    UPROPERTY(BlueprintAssignable, Category = "UI")
    FOnNotationUpdated OnNotationUpdated;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputAction* EnterRotateAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputAction* KeyBindingPressAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    class UInputAction* DeleteEquipmentAction;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
    ECustomizingState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMultiplier")
    float RotationSpeed = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMultiplier")
    float MouseSensitivity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
    FName TargetLevelName;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* MainCamera;

private:
    FVector DefaultCameraOffset;
    FVector CurrentCameraOffset;
    bool bIsZoomedIn = false;
    FRotator DefaultCameraRotation;
    FString CurrentNotation;

    bool bIsDragging;
    FVector2D MouseStartLocation;
    TSoftObjectPtr<UTGCustomizingComponent> MyCustomizingComponent;
    TSoftObjectPtr<UTGEquipmentManager> EquipmentManager;

};
