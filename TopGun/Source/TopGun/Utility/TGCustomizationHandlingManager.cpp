// Fill out your copyright notice in the Description page of Project Settings.


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

USkeletalMesh* UTGCustomizationHandlingManager::GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset)
{
    return UTGModuleSystem::GetMergeCharacterParts(WholeModuleData, ModuleDataAsset.Get());
}


void UTGCustomizationHandlingManager::GenerateModuleButtons(UScrollBox* TargetPanel) const
{
    
}

AActor* UTGCustomizationHandlingManager::SpawnEquip(FName EquipID, APlayerController* Player)
{
    UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(Player->GetWorld()->GetGameInstance());
    UBlueprintGeneratedClass* EquipmentClass = Cast<UBlueprintGeneratedClass>(GameInstance->GetEquipmentManager()->GetEquipClassByID(EquipID));
    if (!EquipmentClass)
    {
       UE_LOG(LogTemp, Error, TEXT("UTGCustomizingComponent::SpawnEquip/EquipmentClass not found."));
       return nullptr;
    }
    FActorSpawnParameters SpawnParameters;
    
    return Player->GetWorld()->SpawnActor<AActor>(EquipmentClass);
}



void UTGCustomizationHandlingManager::SpawnCurrentEquip(FName EquipID, APlayerController* Player)
{
    AActor* TempActor = SpawnEquip(EquipID, Player);
    if(!TempActor)
       return;
    CurrentSpawnedActor = TempActor;
    if (!CurrentSpawnedActor->IsValidLowLevel())
    {
       UE_LOG(LogTemp, Error, TEXT("UTGCustomizingComponent::SpawnCurrentEquip/Failed to spawn Equipment."));
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
       UE_LOG(LogTemp, Error, TEXT("UTGCustomizingComponent::SpawnCurrentEquip/Spawned actor does not implement ITGEquipInterface."));
    }
}



void UTGCustomizationHandlingManager::SpawnModule(FName WeaponID, APlayerController* Player) const
{
    UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(Player->GetWorld()->GetGameInstance());

    ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
    if (MyCharacter)
    {
       const FMeshCategoryData* TargetData = GameInstance->ModuleDataAsset->BaseMeshComponent.Find(WeaponID);
       GameInstance->ModuleBodyPartIndex[TargetData->Category] = WeaponID;
       UClass* AnimClass = MyCharacter->GetMesh()->GetAnimClass();
       USkeleton* Skeleton = MyCharacter->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
       USkeletalMesh* MergedMesh = GetMergedCharacterParts(GameInstance->ModuleBodyPartIndex, GameInstance->ModuleDataAsset);
       if (MergedMesh == nullptr)
       {
          UE_LOG(LogTemp, Error, TEXT("Failed to merge Mesh."));
          return;
       }
       MergedMesh->USkeletalMesh::SetSkeleton(Skeleton);
       MyCharacter->GetMesh()->SetSkeletalMesh(MergedMesh);
       MyCharacter->GetMesh()->SetAnimInstanceClass(AnimClass);
    }
}

void UTGCustomizationHandlingManager::AlterModuleComponent(FName WeaponID, APlayerController* Player)
{
    UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(Player->GetWorld()->GetGameInstance());

    if(GameInstance && GameInstance->ModuleDataAsset)
    {
       ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
       const FMeshCategoryData* TargetData = GameInstance->ModuleDataAsset->BaseMeshComponent.Find(WeaponID);
       GameInstance->ModuleBodyPartIndex[TargetData->Category] = WeaponID;
       MyCharacter->CharacterPartsMap[TargetData->Category]->SetSkeletalMesh(GameInstance->ModuleDataAsset->GetMeshByID(WeaponID));
    } else
    {
       UE_LOG(LogTemp, Error, TEXT("AlterModuleComponent:: id : %s, null"), *WeaponID.ToString());
    }
}

bool UTGCustomizationHandlingManager::AttachActor(APlayerController* Player) const
{
    USceneComponent* MySkeletalMeshComponent = Player->GetCharacter()->GetMesh();
    AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(CurrentSpawnedActor->GetClass(), MySkeletalMeshComponent->GetComponentLocation(), FRotator::ZeroRotator);
    ClonedActor->AttachToComponent(MySkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
    ClonedActor->SetActorEnableCollision(true);
    if(!EquipRegister(ClonedActor, Player))
    {
       UE_LOG(LogTemp, Error, TEXT("Failed to register equipment."));
    }
    
    if (!ClonedActor)
    {
       UE_LOG(LogTemp, Error, TEXT("Failed to spawn cloned actor."));
       return false;
    }
    return true;
}


bool UTGCustomizationHandlingManager::EquipRegister(AActor* ClonedActor, APlayerController* Player) const
{
    UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(Player->GetWorld()->GetGameInstance());

    if (ClonedActor->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()) && CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()))
    {
       const FName TempEquipID = ITGBaseEquipmentInterface::Execute_GetEquipmentID(CurrentSpawnedActor.Get());
       UE_LOG(LogTemp, Warning, TEXT("Registering EquipID: %s"), *TempEquipID.ToString());
       const ETGEquipmentCategory TempEquipCategory = ITGBaseEquipmentInterface::Execute_GetCategory(CurrentSpawnedActor.Get());
       ITGBaseEquipmentInterface::Execute_SetEquipmentID(ClonedActor, TempEquipID);
       ITGBaseEquipmentInterface::Execute_SetBoneID(ClonedActor, CurrentTargetBone);
       const FAttachedActorData TempData(TempEquipID, ClonedActor->GetActorRotation());
       FEquipmentKey TargetKey (CurrentTargetBone, TempEquipCategory,TempEquipID);
       GameInstance->GetEquipmentManager()->SetEquipActorData(TargetKey, TempData);
       return true;
    }
    else
    {
       UE_LOG(LogTemp, Error, TEXT("Spawned actor does not implement ITGWeaponInterface."));
       return false;
    }
}

void UTGCustomizationHandlingManager::RemoveEquipFromCharacter(AActor* EquipToRemove, APlayerController* Player) const
{
    UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(Player->GetWorld()->GetGameInstance());

    if (EquipToRemove)
    {
       if (EquipToRemove->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()))
       {
          GameInstance->GetEquipmentManager()->RemoveFromEquipActorsMap(GameInstance->GetEquipmentManager()->GetKeyForActor(EquipToRemove));
       }

       EquipToRemove->Destroy();
       EquipToRemove = nullptr;
    }
}


void UTGCustomizationHandlingManager::UpdateWeaponActorPosition(const FVector& WorldLocation, const FVector& WorldDirection) const
{
    FVector PlaneNormal = FVector(1, 0, 0);
    FVector PlanePoint = FVector(0, 0, 100);
    FVector FarPoint = WorldLocation + WorldDirection * 10000;
    FVector IntersectionPoint = FMath::LinePlaneIntersection(WorldLocation, FarPoint, PlanePoint, PlaneNormal);

    if (CurrentSpawnedActor->IsValidLowLevel())
    {
       CurrentSpawnedActor->SetActorLocation(IntersectionPoint);
    } else
    {
       UE_LOG(LogTemp, Warning, TEXT("Current SpawnedActor Missing."));
    }
}

bool UTGCustomizationHandlingManager::IsEquipNearBone(APlayerController* Player)
{
    USkeletalMeshComponent* MySkeletalMeshComponent = Player->GetCharacter()->GetMesh();
    if (!MySkeletalMeshComponent || !CurrentSpawnedActor->IsValidLowLevel())
    {
       FPlatformMisc::RequestExit(false);
       return false;
    }

    FVector targetLocation = CurrentSpawnedActor->GetActorLocation();
    FVector closestBoneLocation;
    FName closestBoneName;

    if (MySkeletalMeshComponent)
    {
       closestBoneName = MySkeletalMeshComponent->FindClosestBone(targetLocation, &closestBoneLocation);
    }
    else
    {
       UE_LOG(LogTemp, Warning, TEXT("MySkeletalMeshComponent is null in IsEquipNearBone"));
       return false;
    }

    float closestBoneDistance = FVector::Distance(targetLocation, closestBoneLocation);
    return IsWithinSnapDistance(closestBoneDistance, closestBoneLocation, closestBoneName, Player);
}


bool UTGCustomizationHandlingManager::IsWithinSnapDistance(float distance, const FVector& boneLocation, FName boneName, APlayerController* Player)
{
    if (distance < SnapCheckDistance)
    {
       return SnapActor(boneLocation, distance, boneName, Player);
    }
    return false;
}

void UTGCustomizationHandlingManager::UnSnapActor()
{
    CurrentSpawnedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

bool UTGCustomizationHandlingManager::SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName, APlayerController* Player)
{
    USkeletalMeshComponent* MySkeletalMeshComponent = Player->GetCharacter()->GetMesh();
    CurrentSpawnedActor->AttachToComponent(MySkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
    CurrentTargetBone = ClosestBoneName;
    UE_LOG(LogTemp, Log, TEXT("Found Spot! Bone :%s"), *ClosestBoneName.ToString());
    return true;
}

void UTGCustomizationHandlingManager::HighlightSelectedActor(bool bEnable)
{
    if (bIsHighlighted == bEnable)
       return;

    bIsHighlighted = bEnable;

    if (!CurrentSpawnedActor->IsValidLowLevel())
       return;

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

void UTGCustomizationHandlingManager::SaveRotationData(APlayerController* Player) const
{
    UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(Player->GetWorld()->GetGameInstance());

       const FName TargetID = ITGBaseEquipmentInterface::Execute_GetBoneID(CurrentSelectedActor.Get());
       FAttachedActorData FoundActorData;
       if (GameInstance->GetEquipmentManager()->GetEquipActorData(GameInstance->GetEquipmentManager()->GetKeyForActor(CurrentSelectedActor.Get()), FoundActorData))
       {
          FoundActorData.Rotation = CurrentSelectedActor->GetActorRotation();
          GameInstance->GetEquipmentManager()->SetEquipActorData(GameInstance->GetEquipmentManager()->GetKeyForActor(CurrentSpawnedActor.Get()), FoundActorData);
       }
       else
       {
          UE_LOG(LogTemp, Warning, TEXT("Actor data not found for TargetID: %s"), *TargetID.ToString());
       }
}

void UTGCustomizationHandlingManager::ResetHoldingData()
{
    if (CurrentSpawnedActor->IsValidLowLevel())
    {
       CurrentSpawnedActor->Destroy();
       CurrentTargetBone = FName();
       CurrentSpawnedActor = nullptr;
       CurrentSelectedActor = nullptr;
    }
}

void UTGCustomizationHandlingManager::SetTargetActorRotation(FQuat Rotation) const
{
   if(CurrentSelectedActor->IsValidLowLevel())
   {
      CurrentSelectedActor->AddActorLocalRotation(Rotation, false, nullptr, ETeleportType::None);
   }
}

bool UTGCustomizationHandlingManager::SetCurrentSelectedActor(AActor* TargetActor)
{
   if(TargetActor)
   {
      CurrentSelectedActor = TargetActor;
      return true;
   } else
   {
      return false;
   }
}

AActor* UTGCustomizationHandlingManager::GetCurrentSelectedActor() const
{
   if(CurrentSelectedActor->IsValidLowLevel())
   {
      return CurrentSelectedActor.Get();
   } else
   {
      return nullptr;
   }
}

void UTGCustomizationHandlingManager::DrawDebugHighlight() const
{
   if (CurrentSelectedActor->IsValidLowLevel())
   {
      FVector ActorLocation = CurrentSelectedActor->GetActorLocation();
      float SphereRadius = 2.0f; 
      FColor SphereColor = FColor::Red; 
      DrawDebugSphere(GetWorld(), ActorLocation, SphereRadius, 12, SphereColor, false, -1.0f, 0, 2.0f);
   }
}