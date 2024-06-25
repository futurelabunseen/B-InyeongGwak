// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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
};
