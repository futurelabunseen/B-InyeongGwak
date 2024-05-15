// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Utility/TGModuleSystem.h"
#include "TGGameInstance.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FAttachedActorData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName ActorID;

	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation;

	FAttachedActorData() {}
	FAttachedActorData(FName InActorID, const FRotator& InRotation)
		: ActorID(InActorID), Rotation(InRotation) {}
};


UCLASS()
class TOPGUN_API UTGCGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;
	void ChangeLevel(FName LevelName) const;
	virtual void Shutdown() override;
	TMap<E_PartsCode, FName> ModuleBodyPartIndex;
	TMap<FName , FAttachedActorData> AttachedActorsMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
	class UTGWeaponDataAsset* WeaponDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
	class UTGModuleDataAsset* ModuleDataAsset;
};

