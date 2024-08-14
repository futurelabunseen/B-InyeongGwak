#pragma once

#include "CoreMinimal.h"
#include "Interface/TGCustomizingInterface.h"
#include "Interface/TGCustomizingPlayerInterface.h"
#include "Utility/TGCustomizationHandlingManager.h"
#include "TGCustomizingStateManager.generated.h"

class UTGCustomizingComponent;
class UTGCustomizingUIManager;
class AActor;

UENUM(BlueprintType)
enum class ECustomizingState : uint8
{
	Idle,
	OnDragActor,
	OnSnappedActor,
	OnSelectActor,
	OnRotateEquip,
	OnBindKey,
};

UCLASS()
class TOPGUN_API UTGCustomizingStateManager : public UObject, public ITGCustomizingInterface
{
	GENERATED_BODY()

public:
	UTGCustomizingStateManager();
	void SetPlayerController(ITGCustomizingPlayerInterface* InPlayerController,  TWeakObjectPtr<UTGCustomizationHandlingManager> InCustomizingComponent);
	
	virtual void EnterIdleState() override;
	virtual void EnterDragState() override;
	virtual void EnterSnappedState() override;
	virtual void EnterRotateState() override;
	virtual void EnterSelectActorState() override;
	virtual void EnterBindKeyState() override;
	virtual void ReturnToIdleState(APlayerController* Player) override;
	virtual void HandleEquipSelect(FName WeaponID, APlayerController* Player) override;
	
	
	virtual void UpdateState(APlayerController* Player) override;
	virtual void ReturnToSelectActorState() override;

	virtual void HandleOnPressKeyBindingEquipment() override;
	virtual void HandleOnPressEnterRotateEquipment() override;
	virtual void HandleOnPressDeleteEquipmentAction(APlayerController* Player) override;
	
	virtual void HandleRightMouseClick(APlayerController* Player) override;
	virtual void HandleLeftMouseClick(APlayerController* Player) override;
	virtual void HandleEnterRotateEquipment() override;
	virtual void HandleKeyBindingEquipment() override;
	virtual void HandleDeleteEquipmentAction(APlayerController* Player) override;
	virtual void HandleRotateAction(const FInputActionValue& Value) override;
	virtual void HandleTryFindSelectActor(AActor* HitActor) override;
	virtual void HandleRemoveActorInDesiredPosition(APlayerController* Player) override;
	virtual void HandleProcessPlayerInput(APlayerController* PC) override;
	virtual void HandleKeyBindingInput(const FKey& Key) override;

	ECustomizingState GetCurrentState() const { return CurrentState; }

private:
	void HandleIdleState();
	void HandleDragState(APlayerController* Player);
	void HandleSnappedState();
	void HandleRotateState();
	void HandleSelectActorState();
	void HandleBindKeyState();

	UPROPERTY()
	TWeakObjectPtr<UTGCustomizationHandlingManager> CustomizingComponent;
	
	ITGCustomizingPlayerInterface* PlayerControllerInterface;
	
	ECustomizingState CurrentState;
};