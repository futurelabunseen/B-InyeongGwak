// TGWeaponInterface.h
#pragma once

#include "CoreMinimal.h"
#include "TGBaseEquipmentInterface.h"
#include "TGWeaponInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UTGWeaponInterface : public UTGBaseEquipmentInterface
{
	GENERATED_BODY()
};

class TOPGUN_API ITGWeaponInterface : public ITGBaseEquipmentInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void FunctionFireWeapon(bool FiringHeld, bool HitScan, bool ReadyFiring, USceneComponent* CameraComponent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetDefaultRotation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetRotation(const FQuat& QuatRotation);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVector GetArrowForwardVector() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FQuat GetDefaultRoationQuat() const;
    
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FQuat GetAimingRotation(const FVector& TargetVector) const;
};