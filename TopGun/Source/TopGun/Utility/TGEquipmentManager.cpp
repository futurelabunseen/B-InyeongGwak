// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/TGEquipmentManager.h"

#include "Data/EquipmentData.h"
#include "Interface/TGBaseEquipmentInterface.h"

UTGEquipmentManager::UTGEquipmentManager()
{
}

void UTGEquipmentManager::SetEquipActorData(const FEquipmentKey& Key, const FAttachedActorData& ActorData)
{
	AttachedEquipActorsMap.Add(Key, ActorData);
	BroadcastTotalStats();
}

bool UTGEquipmentManager::GetEquipActorData(const FEquipmentKey& Key, FAttachedActorData& OutActorData) const
{
	if (const FAttachedActorData* FoundData = AttachedEquipActorsMap.Find(Key))
	{
		OutActorData = *FoundData;
		return true;
	}
	return false;
}

bool UTGEquipmentManager::RemoveFromEquipActorsMap(const FEquipmentKey& Key)
{
	bool bRemoved = AttachedEquipActorsMap.Remove(Key) > 0;
	
	if (bRemoved)
	{
		BroadcastTotalStats();
	}
	
	return bRemoved;
}

int32 UTGEquipmentManager::GetEquipmentStats(FName ActorID) const
{
	return 0;
	/*
	if(!EquipDataAsset || !EquipDataAsset->FindRow<FEquipmentData>(ActorID, TEXT("EquipmentLookup")))
		return 0;
	else
	{
		return EquipDataAsset->FindRow<FEquipmentData>(ActorID, TEXT("EquipmentLookup"))->points;
	}
	*/
}

void UTGEquipmentManager::CalculateStatsForEquip(int32& OutTotalPoints, ETGEquipmentCategory Category) const
{
	OutTotalPoints = 0;
	if(AttachedEquipActorsMap.IsEmpty())
		return;
	for (const auto& Pair : AttachedEquipActorsMap)
	{
		if (Pair.Key.Category == Category)
		{
			const FName& EquipmentID = Pair.Key.ActorID;
		

			OutTotalPoints += GetEquipmentStats(EquipmentID);
		}
	}
}

void UTGEquipmentManager::BroadcastTotalStats() const
{
	int32 TotalAttack = 0;
	int32 TotalDefense = 0;
	CalculateStatsForEquip(TotalAttack, ETGEquipmentCategory::Weapon);
	CalculateStatsForEquip(TotalDefense, ETGEquipmentCategory::Armour);
	OnChangeStat.Broadcast(TotalAttack, TotalDefense);
}

void UTGEquipmentManager::SetEquipDataAsset(UDataTable* Data)
{
	EquipDataAsset = Data;
}

void UTGEquipmentManager::CalculateTotalStats(int32& OutTotalAttack, int32& OutTotalDefense) const
{
	int32 TotalAttack = 0;
	int32 TotalDefense = 0;
	CalculateStatsForEquip(TotalAttack, ETGEquipmentCategory::Weapon);
	CalculateStatsForEquip(TotalDefense, ETGEquipmentCategory::Armour);
	UE_LOG(LogTemp, Log, TEXT("Total Attack: %d, Total Defense: %d"), TotalAttack, TotalDefense);
}

void UTGEquipmentManager::EmptyEquipMap()
{
	AttachedEquipActorsMap.Empty();
}

TMap<FEquipmentKey, FAttachedActorData> UTGEquipmentManager::GetEntireAttachedEquipActorMap()
{
	return AttachedEquipActorsMap;
}


TArray<TPair<FName, FEquipmentData>> UTGEquipmentManager::GetEquipmentDataForCategory(ETGEquipmentCategory category) const
{
	TArray<TPair<FName, FEquipmentData>> ResultArray;

	if (!EquipDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipDataAsset is null"));
		return ResultArray;
	}

	EquipDataAsset->ForeachRow<FEquipmentData>(TEXT("EquipmentProcessing"),
		[&ResultArray, category](const FName& Key, const FEquipmentData& EquipData)
		{
			if (EquipData.EquipmentCategory == category)
			{
				ResultArray.Add(TPair<FName, FEquipmentData>(Key, EquipData));
			}
		});

	return ResultArray;
}

UBlueprintGeneratedClass* UTGEquipmentManager::GetEquipClassByID(FName EquipID) const
{
	if (!EquipDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipment data Asset null"));
		return nullptr;
	}
	UE_LOG(LogTemp, Warning, TEXT("GetEquipClassByID ::  EquipID: %s"), *EquipID.ToString());
	const FEquipmentData* EquipData = EquipDataAsset->FindRow<FEquipmentData>(EquipID, TEXT("DEBUG:PlayerCharacter_EquipItem"));
	if (EquipData)
	{
		UBlueprintGeneratedClass* FoundWeaponClass = EquipData->BaseEquipmentClass;
		return FoundWeaponClass;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipment data not found for ID: %s"), *EquipID.ToString());
		return nullptr;
	}
}

FEquipmentKey UTGEquipmentManager::GetKeyForActor(AActor* ClonedActor)
{
	if(ClonedActor->Implements<UTGBaseEquipmentInterface>())
	{
		FName BoneID = ITGBaseEquipmentInterface::Execute_GetBoneID(ClonedActor);
		ETGEquipmentCategory Category = ITGBaseEquipmentInterface::Execute_GetCategory(ClonedActor);
		FName ActorID = ITGBaseEquipmentInterface::Execute_GetEquipmentID(ClonedActor);
	
		return FEquipmentKey(BoneID, Category, ActorID);
	} else
	{
		UE_LOG(LogTemp, Log, TEXT("UTGEquipmentManager::GetKeyForActor Target Actor does not implement EquipInterface"));
		return FEquipmentKey::InvalidKey();
	}
}


