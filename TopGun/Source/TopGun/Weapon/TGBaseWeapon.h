// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/TGWeaponInterface.h"
#include "TGBaseWeapon.generated.h"

UCLASS()
class TOPGUN_API ATGBaseWeapon : public AActor, public ITGWeaponInterface
{
	GENERATED_BODY()
};
