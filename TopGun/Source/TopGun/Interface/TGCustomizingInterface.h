#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InputActionValue.h" 
#include "InputCoreTypes.h" 
#include "TGCustomizingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTGCustomizingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPGUN_API ITGCustomizingInterface
{
	GENERATED_BODY()
	
public:
	virtual void EnterIdleState() = 0;
	virtual void EnterDragState() = 0;
	virtual void EnterSnappedState() = 0;
	virtual void EnterRotateState() = 0;
	virtual void EnterSelectActorState() = 0;
	virtual void EnterBindKeyState() = 0;
	virtual void ReturnToIdleState(APlayerController* Player) = 0;
	virtual void HandleEquipSelect(FName WeaponID, APlayerController* Player) = 0;
	virtual void UpdateState(APlayerController* Player) = 0;
	virtual void ReturnToSelectActorState() = 0;

	virtual void HandleOnPressKeyBindingEquipment() = 0;
	virtual void HandleOnPressEnterRotateEquipment() = 0;
	virtual void HandleOnPressDeleteEquipmentAction(APlayerController* Player) = 0;
    
	virtual void HandleRightMouseClick(APlayerController* Player) = 0;
	virtual void HandleLeftMouseClick(APlayerController* Player) = 0;
	virtual void HandleEnterRotateEquipment() = 0;
	virtual void HandleKeyBindingEquipment() = 0;
	virtual void HandleDeleteEquipmentAction(APlayerController* Player) = 0;
	virtual void HandleRotateAction(const FInputActionValue& Value) = 0;
	virtual void HandleTryFindSelectActor(AActor* HitActor) = 0;
	virtual void HandleRemoveActorInDesiredPosition(APlayerController* Player) = 0;
	virtual void HandleProcessPlayerInput(APlayerController* PC) = 0;
	virtual void HandleKeyBindingInput(const FKey& Key) = 0;
};
