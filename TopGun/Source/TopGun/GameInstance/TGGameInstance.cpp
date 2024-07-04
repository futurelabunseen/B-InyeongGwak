#include "TGGameInstance.h"

#include "Armour/TGBaseArmour.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/TGArmoursDataAsset.h"
#include "Utility/TGModuleSystem.h"
#include "Utility/TGWeaponDataAsset.h"
#include "Weapon/TGBaseWeapon.h"

void UTGCGameInstance::Init()
{
	Super::Init();
	ModuleBodyPartIndex.Add(E_PartsCode::Head, FName("HeadA"));
	ModuleBodyPartIndex.Add(E_PartsCode::UpperBody, FName("BodyA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LowerBody, FName("LegA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LeftHand, FName("LeftArmA"));
	ModuleBodyPartIndex.Add(E_PartsCode::RightHand, FName("RightArmA"));

	PlayerScore = 0; // Initialize score
}

void UTGCGameInstance::ResetGame()
{
	ModuleBodyPartIndex.Empty();
	AttachedArmourActorsMap.Empty();
	AttachedWeaponActorsMap.Empty();
	ModuleBodyPartIndex.Add(E_PartsCode::Head, FName("HeadA"));
	ModuleBodyPartIndex.Add(E_PartsCode::UpperBody, FName("BodyA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LowerBody, FName("LegA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LeftHand, FName("LeftArmA"));
	ModuleBodyPartIndex.Add(E_PartsCode::RightHand, FName("RightArmA"));

	PlayerScore = 0;
}


void UTGCGameInstance::ChangeLevel(FName LevelName) const
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetLevelName is not set."));
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::OpenLevel(World, LevelName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GetWorld() returned null."));
	}
}

void UTGCGameInstance::Shutdown()
{
	Super::Shutdown();
}

void UTGCGameInstance::OnMonsterDeath()
{
	PlayerScore += 10; 
	UE_LOG(LogTemp, Log, TEXT("Player Score: %d"), PlayerScore);
}

TMap<FName, FAttachedActorData> UTGCGameInstance::GetEntireWeaponActorMap()
{
	return AttachedWeaponActorsMap;
}

bool UTGCGameInstance::RemoveFromWeaponActorsMap(FName BoneID)
{
	bool bRemoved = AttachedWeaponActorsMap.Remove(BoneID) > 0;
	if (bRemoved)
	{
		BroadcastTotalStats();
	}
	return bRemoved;
}

bool UTGCGameInstance::GetWeaponActorData(FName BoneID, FAttachedActorData& OutActorData) const
{
	if (const FAttachedActorData* FoundData = AttachedWeaponActorsMap.Find(BoneID))
	{
		OutActorData = *FoundData;
		return true;
	}
	return false;
}

void UTGCGameInstance::SetWeaponActorData(FName BoneID, const FAttachedActorData& ActorData)
{
	AttachedWeaponActorsMap.Add(BoneID, ActorData);
	BroadcastTotalStats();
}

TMap<FName, FAttachedActorData> UTGCGameInstance::GetEntireArmourActorMap()
{
	return AttachedArmourActorsMap;
}

bool UTGCGameInstance::RemoveFromArmourActorsMap(FName BoneID)
{
	bool bRemoved = AttachedArmourActorsMap.Remove(BoneID) > 0;
	if (bRemoved)
	{
		BroadcastTotalStats();
	}
	return bRemoved;
}

bool UTGCGameInstance::GetArmourActorData(FName BoneID, FAttachedActorData& OutActorData) const
{
	if (const FAttachedActorData* FoundData = AttachedArmourActorsMap.Find(BoneID))
	{
		OutActorData = *FoundData;
		return true;
	}
	return false;
}

void UTGCGameInstance::SetArmourActorData(FName BoneID, const FAttachedActorData& ActorData)
{
	AttachedArmourActorsMap.Add(BoneID, ActorData);
	BroadcastTotalStats();
}
template<typename T>
bool GetClassPropertyValue(const UObject* Object, const FName PropertyName, T& OutValue)
{
    if (!Object)
    {
        return false;
    }

    UClass* Class = Object->GetClass();
    FProperty* Property = Class->FindPropertyByName(PropertyName);
    if (Property && Property->IsA<FNumericProperty>())
    {
        FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property);
        if (NumericProperty->IsInteger())
        {
            OutValue = NumericProperty->GetSignedIntPropertyValue(NumericProperty->ContainerPtrToValuePtr<void>(Object));
            return true;
        }
    }
    return false;
}




void UTGCGameInstance::CalculateWeaponStats(int32& OutTotalAttack) const
{
    OutTotalAttack = 0;
	if(AttachedWeaponActorsMap.IsEmpty())
		return;
    for (const auto& Pair : AttachedWeaponActorsMap)
    {
        const FName& ActorID = Pair.Value.ActorID;
        if (WeaponDataAsset && WeaponDataAsset->BaseWeaponClasses.Contains(ActorID))
        {
        	OutTotalAttack += GetWeaponStats(ActorID);
        }
    }
}



void UTGCGameInstance::CalculateArmourStats(int32& OutTotalDefense) const
{
    OutTotalDefense = 100;
	if(AttachedArmourActorsMap.IsEmpty())
		return;
    for (const auto& Pair : AttachedArmourActorsMap)
    {
        const FName& ActorID = Pair.Value.ActorID;
    	OutTotalDefense += GetArmourStats(ActorID);

    }
}

int32 UTGCGameInstance::GetWeaponStats(FName ActorID) const
{
	const UBlueprintGeneratedClass* WeaponClass = *WeaponDataAsset->BaseWeaponClasses.Find(ActorID);
	if (WeaponClass)
	{
		const ATGBaseWeapon* WeaponDefaultObject = Cast<ATGBaseWeapon>(WeaponClass->GetDefaultObject());
		if (WeaponDefaultObject)
		{
			int32 AttackValue = 0;
			if (GetClassPropertyValue<int32>(WeaponDefaultObject, FName("Attack"), AttackValue))
			{
				return AttackValue;
			}
		}
	}

	return 0;
}

int32 UTGCGameInstance::GetArmourStats(FName ActorID) const
{
	if (ArmourDataAsset && ArmourDataAsset->BaseArmourClass.Contains(ActorID))
	{
		const UBlueprintGeneratedClass* ArmourClass = *ArmourDataAsset->BaseArmourClass.Find(ActorID);
		if (ArmourClass)
		{
			const ATGBaseArmour* ArmourDefaultObject = Cast<ATGBaseArmour>(ArmourClass->GetDefaultObject());
			if (ArmourDefaultObject)
			{
				int32 DefenseValue = 0;
				if (GetClassPropertyValue<int32>(ArmourDefaultObject, FName("Defense"), DefenseValue))
				{
					return DefenseValue;
				}
			}
		}
	}
	return 0;

}


void UTGCGameInstance::BroadcastTotalStats() const
{
    int32 TotalAttack = 0;
    int32 TotalDefense = 0;

    CalculateWeaponStats(TotalAttack);
    CalculateArmourStats(TotalDefense);

    OnChangeStat.Broadcast(TotalAttack, TotalDefense);
}

void UTGCGameInstance::CalculateTotalStats()
{
    int32 TotalAttack = 0;
    int32 TotalDefense = 0;

    CalculateWeaponStats(TotalAttack);
    CalculateArmourStats(TotalDefense);

    // You can use these values as needed or log them
    UE_LOG(LogTemp, Log, TEXT("Total Attack: %d, Total Defense: %d"), TotalAttack, TotalDefense);
}
