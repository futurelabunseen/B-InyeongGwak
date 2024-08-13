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



void UTGEquipmentManager::CalculateStatsForEquip(int32& OutTotalPoints, ETGEquipmentCategory Category) const
{
	OutTotalPoints = 0;
	//TEMP=================================
	if(Category == ETGEquipmentCategory::Armour)
	{
		OutTotalPoints = 50;
	}
	//=====================================
	if(AttachedEquipActorsMap.IsEmpty())
		return;
	for (const auto& Pair : AttachedEquipActorsMap)
	{
		if (Pair.Key.Category == Category)
		{
			const FName& EquipmentID = Pair.Key.ActorID;
			
			
			OutTotalPoints += GetEquipPointsByID(EquipmentID);
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

int32 UTGEquipmentManager::GetEquipPointsByID(FName EquipID) const
{
	if (!EquipDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipment data Asset null"));
		return 0;
	}
	UE_LOG(LogTemp, Warning, TEXT("GetEquipClassByID ::  EquipID: %s"), *EquipID.ToString());
	const FEquipmentData* EquipData = EquipDataAsset->FindRow<FEquipmentData>(EquipID, TEXT("DEBUG:PlayerCharacter_EquipItem"));
	if (EquipData)
	{
		return EquipData->points;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipment data not found for ID: %s"), *EquipID.ToString());
		return 0;
	}
}

FEquipmentKey UTGEquipmentManager::GetKeyForActor(AActor* TargetActor)
{
	if(!TargetActor)
	{
		UE_LOG(LogTemp, Log, TEXT("UTGEquipmentManager::GetKeyForActor Target Actor is null"));
		return FEquipmentKey::InvalidKey();

	}
	if(TargetActor->Implements<UTGBaseEquipmentInterface>())
	{
		FName BoneID = ITGBaseEquipmentInterface::Execute_GetBoneID(TargetActor);
		ETGEquipmentCategory Category = ITGBaseEquipmentInterface::Execute_GetCategory(TargetActor);
		FName ActorID = ITGBaseEquipmentInterface::Execute_GetEquipmentID(TargetActor);
	
		return FEquipmentKey(BoneID, Category, ActorID);
	} else
	{
		UE_LOG(LogTemp, Log, TEXT("UTGEquipmentManager::GetKeyForActor Target Actor does not implement EquipInterface"));
		return FEquipmentKey::InvalidKey();
	}
}

void UTGEquipmentManager::BindKeyToEquipment(const FEquipmentKey& EquipKey, FName KeyName)
{
	FAttachedActorData* ActorData = AttachedEquipActorsMap.Find(EquipKey);
	if (ActorData)
	{
		// 이전에 이 키에 바인딩된 장비가 있다면 언바인드
		for (auto& Pair : AttachedEquipActorsMap)
		{
			if (Pair.Value.KeyMapping == KeyName)
			{
				Pair.Value.KeyMapping = NAME_None;
			}
		}

		ActorData->KeyMapping = KeyName;
		UE_LOG(LogTemp, Log, TEXT("Equipment %s bound to key %s"), *EquipKey.ActorID.ToString(), *KeyName.ToString());
	}
}


AActor* UTGEquipmentManager::GetEquipmentByKeyBinding(FName KeyName) const
{
	for (const auto& Pair : AttachedEquipActorsMap)
	{
		if (Pair.Value.KeyMapping == KeyName)
		{
			// 여기서는 ActorID를 반환합니다. 실제 AActor*를 얻으려면 추가 로직이 필요할 수 있습니다.
			return nullptr; // 임시로 nullptr 반환, 실제 구현에서는 해당 ActorID로 AActor*를 찾아 반환해야 합니다.
		}
	}
	return nullptr;
}

void UTGEquipmentManager::UnbindKey(FName KeyName)
{
	for (auto& Pair : AttachedEquipActorsMap)
	{
		if (Pair.Value.KeyMapping == KeyName)
		{
			Pair.Value.KeyMapping = NAME_None;
			UE_LOG(LogTemp, Log, TEXT("Key %s unbound"), *KeyName.ToString());
			break;
		}
	}
}