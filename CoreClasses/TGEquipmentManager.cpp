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
    bool bRemoved = (AttachedEquipActorsMap.Remove(Key) > 0);
    if (bRemoved)
    {
        BroadcastTotalStats();
    }
    return bRemoved;
}

void UTGEquipmentManager::CalculateStatsForEquip(int32& OutTotalPoints, ETGEquipmentCategory Category) const
{
    OutTotalPoints = 0;
    
    // TEMP
    if (Category == ETGEquipmentCategory::Armour)
    {
        OutTotalPoints = 50;
    }
    
    if (AttachedEquipActorsMap.IsEmpty())
    {
        return;
    }
    
    for (const auto& Pair : AttachedEquipActorsMap)
    {
        if (Pair.Key.Category == Category)
        {
            OutTotalPoints += GetEquipPointsByID(Pair.Key.ActorID);
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

TArray<TPair<FName, FEquipmentData>> UTGEquipmentManager::GetEquipmentDataForCategory(ETGEquipmentCategory Category) const
{
    TArray<TPair<FName, FEquipmentData>> ResultArray;
    if (!EquipDataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetEquipmentDataForCategory: EquipDataAsset is null"));
        return ResultArray;
    }

    EquipDataAsset->ForeachRow<FEquipmentData>(TEXT("EquipmentProcessing"),
        [&ResultArray, Category](const FName& Key, const FEquipmentData& EquipData)
        {
            if (EquipData.EquipmentCategory == Category)
            {
                ResultArray.Add(TPair<FName, FEquipmentData>(Key, EquipData));
            }
        });
    return ResultArray;
}

UBlueprintGeneratedClass* UTGEquipmentManager::GetEquipClassByID(const FName& EquipID) const
{
    if (!EquipDataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetEquipClassByID: Equipment data Asset is null"));
        return nullptr;
    }
    UE_LOG(LogTemp, Warning, TEXT("GetEquipClassByID :: EquipID: %s"), *EquipID.ToString());
    const FEquipmentData* EquipData = EquipDataAsset->FindRow<FEquipmentData>(EquipID, TEXT("DEBUG:PlayerCharacter_EquipItem"));
    if (EquipData)
    {
        return EquipData->BaseEquipmentClass;
    }
    UE_LOG(LogTemp, Warning, TEXT("GetEquipClassByID: Equipment data not found for ID: %s"), *EquipID.ToString());
    return nullptr;
}

int32 UTGEquipmentManager::GetEquipPointsByID(const FName& EquipID) const
{
    if (!EquipDataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetEquipPointsByID: Equipment data Asset is null"));
        return 0;
    }
    UE_LOG(LogTemp, Warning, TEXT("GetEquipPointsByID :: EquipID: %s"), *EquipID.ToString());
    const FEquipmentData* EquipData = EquipDataAsset->FindRow<FEquipmentData>(EquipID, TEXT("DEBUG:PlayerCharacter_EquipItem"));
    if (EquipData)
    {
        return EquipData->points;
    }
    UE_LOG(LogTemp, Warning, TEXT("GetEquipPointsByID: Equipment data not found for ID: %s"), *EquipID.ToString());
    return 0;
}

FEquipmentKey UTGEquipmentManager::GetKeyForActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Log, TEXT("GetKeyForActor: Target Actor is null"));
        return FEquipmentKey::InvalidKey();
    }
    if (TargetActor->Implements<UTGBaseEquipmentInterface>())
    {
        FName BoneID = ITGBaseEquipmentInterface::Execute_GetBoneID(TargetActor);
        ETGEquipmentCategory Category = ITGBaseEquipmentInterface::Execute_GetCategory(TargetActor);
        FName ActorID = ITGBaseEquipmentInterface::Execute_GetEquipmentID(TargetActor);
        return FEquipmentKey(BoneID, Category, ActorID);
    }
    UE_LOG(LogTemp, Log, TEXT("GetKeyForActor: Target Actor does not implement Equipment Interface"));
    return FEquipmentKey::InvalidKey();
}

void UTGEquipmentManager::BindKeyToEquipment(const FEquipmentKey& EquipKey, const FName& KeyName)
{
    if (FAttachedActorData* ActorData = AttachedEquipActorsMap.Find(EquipKey))
    {
        for (auto& Pair : AttachedEquipActorsMap)
        {
            if (Pair.Value.KeyMapping == KeyName)
            {
                Pair.Value.KeyMapping = NAME_None;
            }
        }
        ActorData->KeyMapping = KeyName;
        UE_LOG(LogTemp, Log, TEXT("BindKeyToEquipment: Equipment %s bound to key %s"),
               *EquipKey.ActorID.ToString(), *KeyName.ToString());
    }
}

AActor* UTGEquipmentManager::GetEquipmentByKeyBinding(const FName& KeyName) const
{
    for (const auto& Pair : AttachedEquipActorsMap)
    {
        if (Pair.Value.KeyMapping == KeyName)
        {
            return nullptr;
        }
    }
    return nullptr;
}

void UTGEquipmentManager::UnbindKey(const FName& KeyName)
{
    for (auto& Pair : AttachedEquipActorsMap)
    {
        if (Pair.Value.KeyMapping == KeyName)
        {
            Pair.Value.KeyMapping = NAME_None;
            UE_LOG(LogTemp, Log, TEXT("UnbindKey: Key %s unbound"), *KeyName.ToString());
            break;
        }
    }
}
