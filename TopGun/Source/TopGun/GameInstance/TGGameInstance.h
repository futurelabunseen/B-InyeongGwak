#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Utility/TGModuleSystem.h"
#include "TGGameInstance.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChangeStat, int32 /*TotalAttack*/, int32 /*TotalDefense*/);

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
	void ResetGame();
	void ChangeLevel(FName LevelName) const;
	virtual void Shutdown() override;

	FOnChangeStat OnChangeStat;
	
	TMap<E_PartsCode, FName> ModuleBodyPartIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
	class UTGWeaponDataAsset* WeaponDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
	class UTGModuleDataAsset* ModuleDataAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
	class UTGArmoursDataAsset* ArmourDataAsset;
	
	UPROPERTY(BlueprintReadWrite, Category = "Score")
	int32 PlayerScore;
	
	UFUNCTION()
	void OnMonsterDeath();
	UFUNCTION()
	TMap<FName , FAttachedActorData> GetEntireWeaponActorMap();
	UFUNCTION()
	bool RemoveFromWeaponActorsMap(FName BoneID);
	UFUNCTION()
	bool GetWeaponActorData(FName BoneID, FAttachedActorData& OutActorData) const;
	UFUNCTION()
	void SetWeaponActorData(FName BoneID, const FAttachedActorData& ActorData);
	UFUNCTION()
	TMap<FName , FAttachedActorData> GetEntireArmourActorMap();
	UFUNCTION()
	bool RemoveFromArmourActorsMap(FName BoneID);
	UFUNCTION()
	bool GetArmourActorData(FName BoneID, FAttachedActorData& OutActorData) const;
	UFUNCTION()
	void SetArmourActorData(FName BoneID, const FAttachedActorData& ActorData);
	
	// Unified function to calculate total Attack and Defense for both weapons and armor
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void CalculateTotalStats();

	void BroadcastTotalStats() const;
	void CalculateArmourStats(int32& OutTotalDefense) const;
	int32 GetArmourStats(FName ActorID) const;
	int32 GetWeaponStats(FName ActorID) const;

private:
	TMap<FName , FAttachedActorData> AttachedWeaponActorsMap;
	TMap<FName , FAttachedActorData> AttachedArmourActorsMap;

	// Helper functions to calculate the total stats
	void CalculateWeaponStats(int32& OutTotalAttack) const;

	// Function to broadcast the total stats
};
