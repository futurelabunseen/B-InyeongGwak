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
	void FunctionFireWeapon(bool FiringHeld, bool ReadyFiring, USceneComponent * CameraComponent);
};
