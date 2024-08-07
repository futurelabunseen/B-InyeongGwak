// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/EquipmentData.h"
#include "UObject/NoExportTypes.h"
#include "TGEquipmentManager.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FEquipmentKey
{
	GENERATED_BODY()

	FName BoneID;
	ETGEquipmentCategory Category;
	FName ActorID;
	
	bool operator==(const FEquipmentKey& Other) const
	{
		return BoneID == Other.BoneID && Category == Other.Category;
	}

	friend uint32 GetTypeHash(const FEquipmentKey& Key)
	{
		return HashCombine(GetTypeHash(Key.BoneID), GetTypeHash(static_cast<uint8>(Key.Category)));
	}

	FEquipmentKey()
	   : BoneID(NAME_None)
	   , Category(ETGEquipmentCategory::Weapon)
	   , ActorID(NAME_None)
	{}

	FEquipmentKey(FName InBoneID, ETGEquipmentCategory InCategory, FName InActorID)
	   : BoneID(InBoneID)
	   , Category(InCategory)
	   , ActorID(InActorID)
	{}

	
	static FEquipmentKey InvalidKey()
	{
		UE_LOG(LogTemp, Log, TEXT("InvalidKey"));
		return FEquipmentKey(NAME_None, ETGEquipmentCategory::None, NAME_None);
	}
};

USTRUCT(BlueprintType)
struct FAttachedActorData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName KeyMapping;

	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation;

	FAttachedActorData() {}
	FAttachedActorData(FName InKeyMapping, const FRotator& InRotation)
		: KeyMapping(InKeyMapping), Rotation(InRotation) {}
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChangeStat, int32 /*TotalAttack*/, int32 /*TotalDefense*/);

UCLASS()
class TOPGUN_API UTGEquipmentManager : public UObject
{
	GENERATED_BODY()
	UTGEquipmentManager();
	
public:
	FOnChangeStat OnChangeStat;
	void SetEquipActorData(const FEquipmentKey& Key, const FAttachedActorData& ActorData);
	bool GetEquipActorData(const FEquipmentKey& Key, FAttachedActorData& OutActorData) const;
	bool RemoveFromEquipActorsMap(const FEquipmentKey& Key);
	int32 GetEquipmentStats(FName ActorID) const;
	void CalculateStatsForEquip(int32& OutTotalPoints, ETGEquipmentCategory Category) const;
	void BroadcastTotalStats() const;
	void SetEquipDataAsset(UDataTable* Data);
	void CalculateTotalStats(int32& OutTotalAttack, int32& OutTotalDefense) const;
	TArray<TPair<FName, FEquipmentData>> GetEquipmentDataForCategory(ETGEquipmentCategory category) const;
	UBlueprintGeneratedClass* GetEquipClassByID(FName EquipID) const;
	static FEquipmentKey GetKeyForActor(AActor* ClonedActor);
	void EmptyEquipMap();
	TMap<FEquipmentKey , FAttachedActorData> GetEntireAttachedEquipActorMap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
	class UDataTable* EquipDataAsset;
	
private:
	UPROPERTY()
	TMap<FEquipmentKey , FAttachedActorData> AttachedEquipActorsMap;

};
