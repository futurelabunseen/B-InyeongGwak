#include "Utility/TGCustomizationHandlingManager.h"

#include "TGEquipmentManager.h"
#include "TGModuleDataAsset.h"
#include "TGModuleSystem.h"
#include "Character/TGCustomizingCharacterBase.h"
#include "GameInstance/TGGameInstance.h"
#include "Interface/TGBaseEquipmentInterface.h"

UTGCustomizationHandlingManager::UTGCustomizationHandlingManager()
{
}

// Customizing Features

USkeletalMesh* UTGCustomizationHandlingManager::GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset)
{
    return UTGModuleSystem::GetMergeCharacterParts(WholeModuleData, ModuleDataAsset.Get());
}

void UTGCustomizationHandlingManager::GenerateModuleButtons(UScrollBox* TargetPanel) const
{
    //TODO
}

// Spawn Functions
AActor* UTGCustomizationHandlingManager::SpawnEquip(FName EquipID, APlayerController* Player)
{
    if (!Player) return nullptr;
    
    UTGCGameInstance* GameInstance = GetGameInstance(Player);
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEquip: GameInstance not found."));
        return nullptr;
    }

    UBlueprintGeneratedClass* EquipmentClass = Cast<UBlueprintGeneratedClass>(GameInstance->GetEquipmentManager()->GetEquipClassByID(EquipID));
    if (!EquipmentClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEquip: EquipmentClass for %s not found."), *EquipID.ToString());
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParameters;
    return Player->GetWorld()->SpawnActor<AActor>(EquipmentClass, SpawnParameters);
}

void UTGCustomizationHandlingManager::SpawnCurrentEquip(FName EquipID, APlayerController* Player)
{
    AActor* TempActor = SpawnEquip(EquipID, Player);
    if (!TempActor)
        return;

    CurrentSpawnedActor = TempActor;
    if (!CurrentSpawnedActor->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnCurrentEquip: Failed to spawn Equipment."));
        return;
    }

    CurrentSpawnedActor->SetActorEnableCollision(false);

    if (CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnCurrentEquip::Interface Called -> EquipID: %s"), *EquipID.ToString());
        ITGBaseEquipmentInterface::Execute_SetEquipmentID(CurrentSpawnedActor.Get(), EquipID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnCurrentEquip: Spawned actor does not implement Equipment Interface."));
    }
}

void UTGCustomizationHandlingManager::SpawnModule(FName WeaponID, APlayerController* Player) const
{
    if (!Player) return;
    
    UTGCGameInstance* GameInstance = GetGameInstance(Player);
    if (!GameInstance || !GameInstance->ModuleDataAsset)
        return;

    ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
    if (MyCharacter)
    {
        const FMeshCategoryData* TargetData = GameInstance->ModuleDataAsset->BaseMeshComponent.Find(WeaponID);
        if (!TargetData)
        {
            UE_LOG(LogTemp, Error, TEXT("SpawnModule: TargetData for %s not found."), *WeaponID.ToString());
            return;
        }

        GameInstance->ModuleBodyPartIndex[TargetData->Category] = WeaponID;
        UClass* AnimClass = MyCharacter->GetMesh()->GetAnimClass();
        USkeleton* Skeleton = MyCharacter->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
        USkeletalMesh* MergedMesh = GetMergedCharacterParts(GameInstance->ModuleBodyPartIndex, GameInstance->ModuleDataAsset);
        if (!MergedMesh)
        {
            UE_LOG(LogTemp, Error, TEXT("SpawnModule: Failed to merge Mesh."));
            return;
        }
        MergedMesh->SetSkeleton(Skeleton);
        MyCharacter->GetMesh()->SetSkeletalMesh(MergedMesh);
        MyCharacter->GetMesh()->SetAnimInstanceClass(AnimClass);
    }
}

void UTGCustomizationHandlingManager::AlterModuleComponent(FName WeaponID, APlayerController* Player)
{
    if (!Player) return;

    UTGCGameInstance* GameInstance = GetGameInstance(Player);
    if (GameInstance && GameInstance->ModuleDataAsset)
    {
        ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
        if (!MyCharacter)
            return;

        const FMeshCategoryData* TargetData = GameInstance->ModuleDataAsset->BaseMeshComponent.Find(WeaponID);
        if (TargetData)
        {
            GameInstance->ModuleBodyPartIndex[TargetData->Category] = WeaponID;
            MyCharacter->CharacterPartsMap[TargetData->Category]->SetSkeletalMesh(GameInstance->ModuleDataAsset->GetMeshByID(WeaponID));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AlterModuleComponent: TargetData for %s not found."), *WeaponID.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AlterModuleComponent: ModuleDataAsset is null for %s"), *WeaponID.ToString());
    }
}

// Actor Attachment & Registration
bool UTGCustomizationHandlingManager::AttachActor(APlayerController* Player) const
{
    if (!Player || !CurrentSpawnedActor.IsValid())
        return false;

    USceneComponent* SkeletalMeshComp = GetCharacterMesh(Player);
    if (!SkeletalMeshComp)
        return false;

    AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(CurrentSpawnedActor->GetClass(), SkeletalMeshComp->GetComponentLocation(), FRotator::ZeroRotator);
    if (!ClonedActor)
    {
        UE_LOG(LogTemp, Error, TEXT("AttachActor: Failed to spawn cloned actor."));
        return false;
    }

    ClonedActor->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
    ClonedActor->SetActorEnableCollision(true);

    if (!EquipRegister(ClonedActor, Player))
    {
        UE_LOG(LogTemp, Error, TEXT("AttachActor: Failed to register equipment."));
    }

    return true;
}

bool UTGCustomizationHandlingManager::EquipRegister(AActor* ClonedActor, APlayerController* Player) const
{
    if (!Player || !ClonedActor || !CurrentSpawnedActor.IsValid())
        return false;

    UTGCGameInstance* GameInstance = GetGameInstance(Player);
    if (!GameInstance)
        return false;

    if (ClonedActor->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()) &&
        CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()))
    {
        const FName TempEquipID = ITGBaseEquipmentInterface::Execute_GetEquipmentID(CurrentSpawnedActor.Get());
        UE_LOG(LogTemp, Warning, TEXT("Registering EquipID: %s"), *TempEquipID.ToString());
        const ETGEquipmentCategory TempEquipCategory = ITGBaseEquipmentInterface::Execute_GetCategory(CurrentSpawnedActor.Get());
        ITGBaseEquipmentInterface::Execute_SetEquipmentID(ClonedActor, TempEquipID);
        ITGBaseEquipmentInterface::Execute_SetBoneID(ClonedActor, CurrentTargetBone);
        const FAttachedActorData TempData(TempEquipID, ClonedActor->GetActorRotation());
        FEquipmentKey TargetKey(CurrentTargetBone, TempEquipCategory, TempEquipID);
        GameInstance->GetEquipmentManager()->SetEquipActorData(TargetKey, TempData);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EquipRegister: Spawned actor does not implement Equipment Interface."));
        return false;
    }
}


// Handling Spawned Actor
void UTGCustomizationHandlingManager::RemoveEquipFromCharacter(AActor* EquipToRemove, APlayerController* Player) const
{
    if (!EquipToRemove || !Player) return;

    UTGCGameInstance* GameInstance = GetGameInstance(Player);
    if (!GameInstance)
        return;

    if (EquipToRemove->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()))
    {
        GameInstance->GetEquipmentManager()->RemoveFromEquipActorsMap(GameInstance->GetEquipmentManager()->GetKeyForActor(EquipToRemove));
    }

    EquipToRemove->Destroy();
}

void UTGCustomizationHandlingManager::UpdateWeaponActorPosition(const FVector& WorldLocation, const FVector& WorldDirection) const
{
    if (!CurrentSpawnedActor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateWeaponActorPosition: CurrentSpawnedActor missing."));
        return;
    }

    FVector PlaneNormal = FVector(1, 0, 0);
    FVector PlanePoint = FVector(0, 0, 100);
    FVector FarPoint = WorldLocation + WorldDirection * 10000;
    FVector IntersectionPoint = FMath::LinePlaneIntersection(WorldLocation, FarPoint, PlanePoint, PlaneNormal);

    CurrentSpawnedActor->SetActorLocation(IntersectionPoint);
}

bool UTGCustomizationHandlingManager::IsEquipNearBone(APlayerController* Player)
{
    if (!Player || !CurrentSpawnedActor.IsValid())
    {
        FPlatformMisc::RequestExit(false);
        return false;
    }

    USkeletalMeshComponent* SkeletalMeshComp = GetCharacterMesh(Player);
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("IsEquipNearBone: SkeletalMeshComponent is null."));
        return false;
    }

    FVector TargetLocation = CurrentSpawnedActor->GetActorLocation();
    FVector ClosestBoneLocation;
    FName ClosestBoneName = SkeletalMeshComp->FindClosestBone(TargetLocation, &ClosestBoneLocation);
    float BoneDistance = FVector::Distance(TargetLocation, ClosestBoneLocation);

    return IsWithinSnapDistance(BoneDistance, ClosestBoneLocation, ClosestBoneName, Player);
}

bool UTGCustomizationHandlingManager::IsWithinSnapDistance(float Distance, const FVector& BoneLocation, FName BoneName, APlayerController* Player)
{
    return (Distance < SnapCheckDistance) ? SnapActor(BoneLocation, Distance, BoneName, Player) : false;
}

void UTGCustomizationHandlingManager::UnSnapActor()
{
    if (CurrentSpawnedActor.IsValid())
    {
        CurrentSpawnedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }
}

bool UTGCustomizationHandlingManager::SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName, APlayerController* Player)
{
    if (!Player || !CurrentSpawnedActor.IsValid())
        return false;

    USkeletalMeshComponent* SkeletalMeshComp = GetCharacterMesh(Player);
    CurrentSpawnedActor->AttachToComponent(SkeletalMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
    CurrentTargetBone = ClosestBoneName;
    UE_LOG(LogTemp, Log, TEXT("SnapActor: Found Spot! Bone: %s"), *ClosestBoneName.ToString());
    return true;
}

void UTGCustomizationHandlingManager::HighlightSelectedActor(bool bEnable)
{
    if (bIsHighlighted == bEnable || !CurrentSpawnedActor.IsValid())
        return;

    bIsHighlighted = bEnable;

    TArray<UMeshComponent*> MeshComponents;
    CurrentSpawnedActor->GetComponents<UMeshComponent>(MeshComponents);
    
    for (UMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetRenderCustomDepth(bEnable);
            MeshComp->SetCustomDepthStencilValue(bEnable ? 1 : 0);
        }
    }
}

// Rotation & Selection
void UTGCustomizationHandlingManager::SaveRotationData(APlayerController* Player) const
{
    if (!Player || !CurrentSelectedActor.IsValid())
        return;

    UTGCGameInstance* GameInstance = GetGameInstance(Player);
    if (!GameInstance)
        return;

    const FName TargetID = ITGBaseEquipmentInterface::Execute_GetBoneID(CurrentSelectedActor.Get());
    FAttachedActorData FoundActorData;
    if (GameInstance->GetEquipmentManager()->GetEquipActorData(GameInstance->GetEquipmentManager()->GetKeyForActor(CurrentSelectedActor.Get()), FoundActorData))
    {
        FoundActorData.Rotation = CurrentSelectedActor->GetActorRotation();
        GameInstance->GetEquipmentManager()->SetEquipActorData(GameInstance->GetEquipmentManager()->GetKeyForActor(CurrentSpawnedActor.Get()), FoundActorData);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveRotationData: Actor data not found for TargetID: %s"), *TargetID.ToString());
    }
}

void UTGCustomizationHandlingManager::ResetHoldingData()
{
    if (CurrentSpawnedActor.IsValid())
    {
        CurrentSpawnedActor->Destroy();
        CurrentTargetBone = FName();
        CurrentSpawnedActor = nullptr;
        CurrentSelectedActor = nullptr;
    }
}

void UTGCustomizationHandlingManager::SetTargetActorRotation(FQuat Rotation) const
{
    if (CurrentSelectedActor.IsValid())
    {
        CurrentSelectedActor->AddActorLocalRotation(Rotation, false, nullptr, ETeleportType::None);
    }
}

bool UTGCustomizationHandlingManager::SetCurrentSelectedActor(AActor* TargetActor)
{
    if (TargetActor)
    {
        CurrentSelectedActor = TargetActor;
        return true;
    }
    return false;
}

AActor* UTGCustomizationHandlingManager::GetCurrentSelectedActor() const
{
    return (CurrentSelectedActor.IsValid() ? CurrentSelectedActor.Get() : nullptr);
}

// Debug
void UTGCustomizationHandlingManager::DrawDebugHighlight() const
{
    if (CurrentSelectedActor.IsValid())
    {
        FVector ActorLocation = CurrentSelectedActor->GetActorLocation();
        DrawDebugSphere(GetWorld(), ActorLocation, 2.0f, 12, FColor::Red, false, -1.0f, 0, 2.0f);
    }
}
