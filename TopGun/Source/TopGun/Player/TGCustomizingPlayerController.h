#pragma once

#include "CoreMinimal.h"
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
	OnRotateActor UMETA(DisplayName = "On Snapped Actor")
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
	// Weapon Attach
	void UpdateWeaponActorPosition();
	void RemoveWeaponInDesiredPosition();
	void RemoveWeaponFromCharacter(class ATGBaseWeapon* WeaponToRemove) const;
	void AttachWeapon();
	void CheckWeaponActorProximity();
	void CheckSnappedCancellation();
	AActor* FindTargetActorUnderMouse();
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
	void OnEnterAction();
	void OnRotateWeaponActor();

	// State Management
	void ReturnToIdleState();
	void SaveRotationData() const;
	void ResetHoldingData();

public:
	// UI
	UFUNCTION(BlueprintCallable)
	void AddButtonToPanel(class UScrollBox* TargetPanel, TSubclassOf<class UUserWidget> TargetButtonWidget, FName ID);
	UFUNCTION(BlueprintCallable)
	void AddWeaponButtonToPanel(class UScrollBox* TargetPanel);
	UFUNCTION(BlueprintCallable)
	void AddModuleButtonToPanel(class UScrollBox* TargetPanel);
	UFUNCTION(BlueprintCallable)
	void OnWeaponSelected(FName WeaponID);
	UFUNCTION(BlueprintCallable)
	void OnModuleSelected(FName WeaponID);
	//==VARIABLES========================================================
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
	float SnapCheckDistance = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMultiplier")
	float RotationSpeed = 0.4f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMultiplier")
	float MouseSensitivity = 0.2f;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FName TargetLevelName;

	
private:
	TObjectPtr<class USkeletalMeshComponent> MySkeletalMeshComponent;
	TWeakObjectPtr<class AActor> CurrentSpawnedActor;
	TWeakObjectPtr<class AActor> CurrentRotationSelectedActor;
	FName CurrentTargetBone;
	bool bIsDragging;
	FVector2D MouseStartLocation;
	TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset;
	TWeakObjectPtr<UTGWeaponDataAsset> WeaponDataAsset;
	//TEMP
	void DrawDebugHighlight();
	TWeakObjectPtr<UTGCGameInstance> MyGameInstance;

};

