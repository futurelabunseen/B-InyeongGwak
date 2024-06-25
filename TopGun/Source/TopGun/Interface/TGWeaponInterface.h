// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/SpringArmComponent.h" 
#include "TGWeaponInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTGWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

class TOPGUN_API ITGWeaponInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void FunctionFireWeapon(bool FiringHeld, bool HitScan, bool ReadyFiring, USceneComponent * CameraComponent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetWeaponID(FName WeaponID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetBoneID(FName BoneID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FName GetWeaponID();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FName GetBoneID();

	virtual void InitializeWeapon(FName WeaponID, FName BoneID) = 0;

	virtual void SetDefaultRotation() = 0;

	virtual void SetRotation(const FQuat& QuatRotation) = 0;

	virtual FVector GetArrowForwardVector() const = 0;

	virtual FQuat GetDefaultRoationQuat() const = 0;
	
	virtual FQuat GetAimingRotation(const FVector& TargetVector) const = 0;

	virtual void SetSpringArmComponent(USpringArmComponent* SpringArmComponent) = 0;

	virtual USpringArmComponent* GetSpringArmComponent() const = 0;
};
