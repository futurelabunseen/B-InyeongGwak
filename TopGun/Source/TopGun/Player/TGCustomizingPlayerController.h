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
    void SetCustomizingStateManager(ITGCustomizingInterface* CustomizingStateInterface, TWeakObjectPtr<UTGCustomizationHandlingManager>
                                    CustomizingComponent, TWeakObjectPtr<UTGCustomizingUIManager>
                                    CustomizingUIManager);
protected:
    virtual void BeginPlay() override;
    void OnPressDeleteEquipmentAction();
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;

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
    virtual void ClearCurrentEquipment() override;
    virtual void UpdateEquipActorPosition() override;
    virtual void CheckSnappedCancellation() override;
public:
   
    UFUNCTION(BlueprintCallable)
    void RemoveActorInDesiredPosition();
    virtual void OnRotateCharacter(const FInputActionValue& Value) override;

private:
    // Input Related
    void OnClickRightMouse();
    void OnClickLeftMouse();
    void MouseLeftClickStarted();
    void StartMouseDrag();
    void MouseLeftClickEnded();
    void StopMouseDrag();
    void OnClickEscape();
    void OnRotateAction(const FInputActionValue& Value);
    
    //InputBinding
    bool IsInputKeyDown(const FKey& Key) const;
    FKey GetActionPressedKey(const UInputAction* Action) const;
    virtual void ProcessPlayerInput(float DeltaTime, bool bGamePaused) override;
    void HandleKeyBindingInput(const FKey& Key);
    virtual bool IsValidKeyForBinding(const FKey& Key) const override;


    //Camera Related
public:
   
    void UpdateNotationUI(const FString& NewText);

private:
    void UpdateZoomedCameraPosition();
    virtual void SwitchToZoomedCamera(AActor* FocusActor) override;
    virtual void ReturnToDefaultCamera() override;
    virtual void ClearNotationUI() override;
    virtual void OnRotateCameraZoom(const FInputActionValue& Value) override;
    virtual void FindTargetActorForKeyBind(AActor* CurrentWeapon, const FKey& Key) override;
    
public:
    UFUNCTION(BlueprintCallable)
    void OnEnterAction();
    virtual void OnRotateEquipment() override;
    
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

private:
    FVector DefaultCameraOffset;
    FVector CurrentCameraOffset;
    bool bIsZoomedIn = false;
    FRotator DefaultCameraRotation;
    FString CurrentNotation;
    TWeakObjectPtr<UTGCustomizingUIManager> MyCustimizingUIManager;
    bool bIsDragging;
    FVector2D MouseStartLocation;
    UPROPERTY()
    TWeakObjectPtr<UTGCustomizationHandlingManager> MyCustomizingComponent;
    ITGCustomizingInterface* MyCustomizingStateManagerInterface;
    UPROPERTY()
    TSoftObjectPtr<UTGEquipmentManager> EquipmentManager;
    
};
