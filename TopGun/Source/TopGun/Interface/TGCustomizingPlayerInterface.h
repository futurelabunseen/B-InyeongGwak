// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InputActionValue.h" 
#include "InputCoreTypes.h" 
#include "TGCustomizingPlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTGCustomizingPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPGUN_API ITGCustomizingPlayerInterface
{
	GENERATED_BODY()

public:
	virtual void ClearCurrentEquipment() = 0;
	virtual void SetVisibilityCurrentWeaponToolWidget(bool bVisible) = 0;
	virtual void SwitchToZoomedCamera(AActor* FocusActor) = 0;
	virtual void ReturnToDefaultCamera() = 0;
	virtual void ClearNotationUI() = 0;
	virtual void UpdateEquipActorPosition() = 0;
	virtual void CheckSnappedCancellation() = 0;
	virtual void TryFindSelectActor() = 0;
	virtual void OnRotateEquipment() = 0;
	virtual void OnRotateCharacter(const FInputActionValue& Value) = 0;
	virtual void OnRotateCameraZoom(const FInputActionValue& Value) = 0;
	virtual bool IsValidKeyForBinding(const FKey& Key) const = 0;
	virtual void FindTargetActor(AActor* CurrentWeapon, const FKey& Key) = 0;
};
