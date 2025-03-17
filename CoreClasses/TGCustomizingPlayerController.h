/**
 * ┌────────────────────────────────────────────────────────────────────────────┐
 * │ ATGCustomizingPlayerController                                           
 * │                                                                          
 * │ Customizing 모드에서의 플레이어 입력과 UI/Camera 제어를 담당합니다.       
 * │                                                                         
 * │ 주요 역할:                                                               
 * │  마우스 입력으로 액터를 선택·드래그·스냅·회전하도록 요청               
 * │  StateManager(ITGCustomizingInterface) 인터페이스에 상태 전환 전달     
 * │                                                                          
 * │ 의존성:                                                                  
 * │  MyCustomizingStateManagerInterface: 상태 흐름 제어                    
 * │  MyCustomizingComponent: 스폰·스냅 로직 처리                           
 * │  MyCustimizingUIManager: UI 업데이트                                   
 * └────────────────────────────────────────────────────────────────────────────┘
 */
#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "GameInstance/TGGameInstance.h"
#include "Interface/TGCustomizingInterface.h"
#include "Interface/TGCustomizingPlayerInterface.h"
#include "Utility/TGCustomizingUIManager.h"
#include "TGCustomizingPlayerController.generated.h"

class UTGCustomizationHandlingManager;
class UTGCustomizingUIManager;
class UInputMappingContext;

UCLASS()
class TOPGUN_API ATGCustomizingPlayerController : public APlayerController, public ITGCustomizingPlayerInterface
{
    GENERATED_BODY()

    public:
    ATGCustomizingPlayerController();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotationUpdated, FText, NewNotation);
    UPROPERTY(BlueprintAssignable, Category = "UI")
    FOnNotationUpdated OnNotationUpdated;

    UFUNCTION(BlueprintCallable)
    void BindNotationUpdatedEvent(UTextBlock* NotationTextBlock);

    UFUNCTION(BlueprintCallable)
    UTGCustomizingUIManager* GetCustomizingUIManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWidget")
    TSubclassOf<UUserWidget> InventoryWidgetTemplate;

    UTGCustomizationHandlingManager* GetMyCustomizingComponent();

    void SetCustomizingStateManager(ITGCustomizingInterface* CustomizingStateInterface, TWeakObjectPtr<UTGCustomizationHandlingManager> CustomizingComponent, TWeakObjectPtr<UTGCustomizingUIManager> CustomizingUIManager);

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void ProcessPlayerInput(float DeltaTime, bool bGamePaused) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void OnPressDeleteEquipmentAction();

private:
    UFUNCTION(BlueprintCallable)
    void OnPressEnterRotateEquipment();
    UFUNCTION(BlueprintCallable)
    void OnPressKeyBindingEquipment();
    UFUNCTION(BlueprintCallable)
    void OnPressReturnToIdleState();

    // Equip Attach
    AActor* FindTargetActorUnderMouse() const;
    virtual void TryFindSelectActor() override;
    virtual void ClearCurrentSpawnedEquip() override;
    virtual void UpdateEquipActorPosition() override;
    virtual void CheckSnappedCancellation() override;

public:
    UFUNCTION(BlueprintCallable)
    void RemoveActorInDesiredPosition();
    virtual void OnRotateCharacter(const FInputActionValue& Value) override;

private:
    // Input related
    void OnClickRightMouse();
    void OnClickLeftMouse();
    void MouseLeftClickStarted();
    void StartMouseDrag();
    void MouseLeftClickEnded();
    void StopMouseDrag();
    void OnClickEscape();
    void OnRotateAction(const FInputActionValue& Value);

    bool IsInputKeyDown(const FKey& Key) const;
    FKey GetActionPressedKey(const UInputAction* Action) const;
    virtual bool IsValidKeyForBinding(const FKey& Key) const override;

    // Camera related
public:
    void UpdateNotationUI(const FString& NewText);

private:
    void UpdateZoomedCameraPosition();
    virtual void SwitchToZoomedCamera(AActor* FocusActor) override;
    virtual void ReturnToDefaultCamera() override;
    virtual void ClearNotationUI() override;
    virtual void FindTargetActorForKeyBind(AActor* CurrentWeapon, const FKey& Key) override;

public:
    UFUNCTION(BlueprintCallable)
    void OnEnterAction();
    virtual void OnRotateEquipment() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputAction* LClickAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputAction* RClickAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputAction* EscapeAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputAction* RotateAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputAction* EnterAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputAction* EnterRotateAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputAction* KeyBindingPressAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
    UInputAction* DeleteEquipmentAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMultiplier")
    float RotationSpeed = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMultiplier")
    float MouseSensitivity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
    FName TargetLevelName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* MainCamera;

    UFUNCTION(BlueprintCallable)
    void OnEquipSelect(FName WeaponID);

private:
    virtual void SetVisibilityCurrentWeaponToolWidget(bool value) override;

    // Camera state
    FVector DefaultCameraOffset;
    FVector CurrentCameraOffset;
    bool bIsZoomedIn = false;
    FRotator DefaultCameraRotation;
    FString CurrentNotation;

    // Customizing components
    TWeakObjectPtr<UTGCustomizingUIManager> MyCustimizingUIManager;
    bool bIsDragging;
    FVector2D MouseStartLocation;
    UPROPERTY()
    TWeakObjectPtr<UTGCustomizationHandlingManager> MyCustomizingComponent;
    ITGCustomizingInterface* MyCustomizingStateManagerInterface;
    UPROPERTY()
    TSoftObjectPtr<UTGEquipmentManager> EquipmentManager;

    // Optimization
    UPROPERTY(EditAnywhere, Category = "Performance")
    float StateUpdateInterval = 0.02f;
    float LastStateUpdateTime = 0.f;
};
