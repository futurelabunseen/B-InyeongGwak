#include "TGCustomizingComponent.h"
#include "Blueprint/UserWidget.h"
#include "Character/TGCustomizingCharacterBase.h"
#include "Components/ScrollBox.h"
#include "UI/TGInventoryWeaponButton.h"
#include "Utility/TGModuleSystem.h"
#include "Utility/TGModuleDataAsset.h"
#include "Utility/TGWeaponDataAsset.h"
#include "GameFramework/Actor.h"
#include "GameInstance/TGGameInstance.h"
#include "Interface/TGArmourInterface.h"
#include "Interface/TGWeaponInterface.h"


UTGCustomizingComponent::UTGCustomizingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTGCustomizingComponent::BeginPlay()
{
	Super::BeginPlay();

	MyGameInstance =  Cast<UTGCGameInstance>(GetWorld()->GetGameInstance());
	if (!MyGameInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast GameInstance to UTGCGameInstance."));
		FGenericPlatformMisc::RequestExit(false);
		return;
	}
	
	WeaponDataAsset = TWeakObjectPtr<UTGWeaponDataAsset>(MyGameInstance->WeaponDataAsset);
	ModuleDataAsset = TWeakObjectPtr<UTGModuleDataAsset>(MyGameInstance->ModuleDataAsset);
	ArmourDataAsset = TWeakObjectPtr<UTGArmoursDataAsset>(MyGameInstance->ArmourDataAsset);
	
	AActor* Owner = GetOwner();
	if (Owner)
	{
		ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(Owner);
		if (MyCharacter)
		{
			MySkeletalMeshComponent = MyCharacter->GetMesh();
			if (!MySkeletalMeshComponent)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to find MySkeletalMeshComponent on the owning character."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Owner is not a valid customizing character."));
		}
	}

	if (!WeaponDataAsset.IsValid() || !ModuleDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("DataAsset is null."));
		FGenericPlatformMisc::RequestExit(false);
		return;
	}
}

USkeletalMesh* UTGCustomizingComponent::GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset)
{
	return UTGModuleSystem::GetMergeCharacterParts(WholeModuleData, ModuleDataAsset.Get());
}

UBlueprintGeneratedClass* UTGCustomizingComponent::GetWeaponClassById(FName WeaponID, class UTGWeaponDataAsset* WeaponDataAsset)
{
	if (!WeaponDataAsset) return nullptr;
	UBlueprintGeneratedClass** FoundWeaponClass = WeaponDataAsset->BaseWeaponClasses.Find(WeaponID);
	return FoundWeaponClass ? *FoundWeaponClass : nullptr;
}

UBlueprintGeneratedClass* UTGCustomizingComponent::GetArmourClassById(FName ArmourID,
	UTGArmoursDataAsset* ArmourDataAsset)
{
	if (!ArmourDataAsset) return nullptr;
	UBlueprintGeneratedClass** FoundArmourClass = ArmourDataAsset->BaseArmourClass.Find(ArmourID);
	return FoundArmourClass ? *FoundArmourClass : nullptr;
}


void UTGCustomizingComponent::GenerateWeaponButtons(UScrollBox* TargetPanel) const
{
	if (!TargetPanel || !WeaponButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters. Module"));
		return;
	}
	for (const TPair<FName, UBlueprintGeneratedClass*>& WeaponPair : WeaponDataAsset->BaseWeaponClasses)
	{
		if (!GetOwner() || !TargetPanel || !WeaponButtonWidgetClass) return;
	
		UWorld* World = GetOwner()->GetWorld();
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, WeaponButtonWidgetClass);
		if (!CreatedWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Created widget null"));
			return;
		}
		if (UTGInventoryWeaponButton* InventoryButton = Cast<UTGInventoryWeaponButton>(CreatedWidget))
		{
			int32 myStat = MyGameInstance->GetWeaponStats(WeaponPair.Key);
			InventoryButton->SetupButton(WeaponPair.Key, myStat, 0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryWeaponButton."));
			return;
		}
		TargetPanel->AddChild(CreatedWidget);
	}
}

void UTGCustomizingComponent::GenerateModuleButtons(UScrollBox* TargetPanel) const
{
	if (!TargetPanel || !ModuleButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters. Module"));
		return;
	}

	for (const TPair<FName, FMeshCategoryData>& TargetMap : ModuleDataAsset->BaseMeshComponent)
	{
		UWorld* World = GetOwner()->GetWorld();
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, ModuleButtonWidgetClass);
		if (!CreatedWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Created widget null"));
			return;
		}
		if (UTGInventoryWeaponButton* InventoryButton = Cast<UTGInventoryWeaponButton>(CreatedWidget))
		{
			InventoryButton->SetupButton(TargetMap.Key, 0, 0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryWeaponButton."));
			return;
		}
		TargetPanel->AddChild(CreatedWidget);
	}
}

void UTGCustomizingComponent::GenerateArmourButtons(UScrollBox* TargetPanel) const
{
	if (!TargetPanel || !ArmourButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters. Module"));
		return;
	}

	for (const TPair<FName, UBlueprintGeneratedClass*>& ArmourPair : ArmourDataAsset->BaseArmourClass)
	{
		if (!GetOwner() || !TargetPanel || !ArmourButtonWidgetClass) return;
	
		UWorld* World = GetOwner()->GetWorld();
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, ArmourButtonWidgetClass);
		if (!CreatedWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Created widget null"));
			return;
		}
		if (UTGInventoryWeaponButton* InventoryButton = Cast<UTGInventoryWeaponButton>(CreatedWidget))
		{
			int32 MyStat = MyGameInstance->GetArmourStats(ArmourPair.Key);
			InventoryButton->SetupButton(ArmourPair.Key, 0, MyStat);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryWeaponButton."));
			return;
		}
		TargetPanel->AddChild(CreatedWidget);
	}
}


AActor* UTGCustomizingComponent::SpawnWeapon(FName WeaponID)
{
	if (!WeaponDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponDataAsset is null."));
		return nullptr;
	}

	UBlueprintGeneratedClass* WeaponClass = GetWeaponClassById(WeaponID, WeaponDataAsset.Get());
	if (!WeaponClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Weapon class not found."));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;

	return GetWorld()->SpawnActor<AActor>(WeaponClass);
}

AActor* UTGCustomizingComponent::SpawnArmour(FName ArmourID)
{
	if (!ArmourDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ArmourDataAsset is null."));
		return nullptr;
	}

	UBlueprintGeneratedClass* ArmourClass = GetArmourClassById(ArmourID, ArmourDataAsset.Get());
	if (!ArmourClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ArmourClass class not found."));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;

	return GetWorld()->SpawnActor<AActor>(ArmourClass);
}



void UTGCustomizingComponent::SpawnCurrentWeapon(FName WeaponID)
{
	CurrentSpawnedActor = SpawnWeapon(WeaponID);
	if (!CurrentSpawnedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn weapon."));
		return;
	}
	
	CurrentSpawnedActor->SetActorEnableCollision(false);
	if (CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()))
	{
		ITGWeaponInterface::Execute_SetWeaponID(CurrentSpawnedActor, WeaponID);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawned actor does not implement ITGWeaponInterface."));
	}
}

void UTGCustomizingComponent::SpawnCurrentArmour(FName ArmourID)
{
	CurrentSpawnedActor = SpawnArmour(ArmourID);
	if (!CurrentSpawnedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn armour."));
		return;
	}
	
	CurrentSpawnedActor->SetActorEnableCollision(false);
	if (CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGArmourInterface::StaticClass()))
	{
		ITGArmourInterface::Execute_SetArmourID(CurrentSpawnedActor, ArmourID);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TGCustomizingComp :: Spawned actor does not implement ITGArmourInterface."));
	}
}


void UTGCustomizingComponent::SpawnModule(FName WeaponID) const
{
	if (!ModuleDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ModuleDataAsset is null."));
		return;
	}

	ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetOwner());
	if (MyCharacter && MyCharacter->CustomizingComponent)
	{
		const FMeshCategoryData* TargetData = ModuleDataAsset->BaseMeshComponent.Find(WeaponID);
		MyGameInstance->ModuleBodyPartIndex[TargetData->Category] = WeaponID;
		UClass* AnimClass = MyCharacter->GetMesh()->GetAnimClass();
		USkeleton* Skeleton = MyCharacter->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
		USkeletalMesh* MergedMesh = MyCharacter->CustomizingComponent->GetMergedCharacterParts(MyGameInstance->ModuleBodyPartIndex, ModuleDataAsset.Get());
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

void UTGCustomizingComponent::AlterModuleComponent(FName WeaponID)
{
	if(MyGameInstance.IsValid() && MyGameInstance->ModuleDataAsset)
	{
		ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetOwner());
		const FMeshCategoryData* TargetData = MyGameInstance->ModuleDataAsset->BaseMeshComponent.Find(WeaponID);
		MyGameInstance->ModuleBodyPartIndex[TargetData->Category] = WeaponID;
		MyCharacter->CharacterPartsMap[TargetData->Category]->SetSkeletalMesh(MyGameInstance->ModuleDataAsset->GetMeshByID(WeaponID));
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("AlterModuleComponent:: id : %s, null"), *WeaponID.ToString());
	}
}




bool UTGCustomizingComponent::AttachActor() const
{
	AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(CurrentSpawnedActor->GetClass(), MySkeletalMeshComponent->GetComponentLocation(), FRotator::ZeroRotator);
	ClonedActor->AttachToComponent(MySkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
	ClonedActor->SetActorEnableCollision(true);
	if (!WeaponRegister(ClonedActor))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to register weapon."));
	}

	if (!ArmourRegister(ClonedActor))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to register armour."));
	}
	
	if (!ClonedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn cloned actor."));
		return false;
	}
    
	return true;
}




bool UTGCustomizingComponent::WeaponRegister(AActor* ClonedActor) const
{
	if (ClonedActor->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()) && CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()))
	{
		const FName TempWeaponID = ITGWeaponInterface::Execute_GetWeaponID(CurrentSpawnedActor);
		ITGWeaponInterface::Execute_SetWeaponID(ClonedActor, TempWeaponID);
		ITGWeaponInterface::Execute_SetBoneID(ClonedActor, CurrentTargetBone);
		const FAttachedActorData TempData(TempWeaponID, ClonedActor->GetActorRotation());
		MyGameInstance->SetWeaponActorData(CurrentTargetBone, TempData);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawned actor does not implement ITGWeaponInterface."));
		return false;
	}
}

bool UTGCustomizingComponent::ArmourRegister(AActor* ClonedActor) const
{
	if (ClonedActor->GetClass()->ImplementsInterface(UTGArmourInterface::StaticClass()) && CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGArmourInterface::StaticClass()))
	{
		const FName TempArmourID = ITGArmourInterface::Execute_GetArmourID(CurrentSpawnedActor);
		ITGArmourInterface::Execute_SetArmourID(ClonedActor, TempArmourID);
		ITGArmourInterface::Execute_SetBoneID(ClonedActor, CurrentTargetBone);
		const FAttachedActorData TempData(TempArmourID, ClonedActor->GetActorRotation());
		MyGameInstance->SetArmourActorData(CurrentTargetBone, TempData);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawned actor does not implement ITGArmourInterface."));
		return false;
	}
}


void UTGCustomizingComponent::RemoveWeaponFromCharacter(AActor* WeaponToRemove) const
{
	if (WeaponToRemove)
	{
		if (WeaponToRemove->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()))
		{
			FName TempBoneID = ITGWeaponInterface::Execute_GetBoneID(WeaponToRemove);
			MyGameInstance->RemoveFromWeaponActorsMap(TempBoneID);
		}

		WeaponToRemove->Destroy();
		WeaponToRemove = nullptr;
	}
}

void UTGCustomizingComponent::RemoveArmourFromCharacter(AActor* ArmourToRemove, FName TempBoneID) const
{
	if (ArmourToRemove)
	{
		if (ArmourToRemove->GetClass()->ImplementsInterface(UTGArmourInterface::StaticClass()))
		{
			MyGameInstance->RemoveFromArmourActorsMap(TempBoneID);
		}
		ArmourToRemove->Destroy();
		ArmourToRemove = nullptr;
	}
}

void UTGCustomizingComponent::UpdateWeaponActorPosition(const FVector& WorldLocation, const FVector& WorldDirection) const
{
	FVector PlaneNormal = FVector(1, 0, 0);
	FVector PlanePoint = FVector(0, 0, 100);
	FVector FarPoint = WorldLocation + WorldDirection * 10000;
	FVector IntersectionPoint = FMath::LinePlaneIntersection(WorldLocation, FarPoint, PlanePoint, PlaneNormal);

	if (CurrentSpawnedActor)
	{
		CurrentSpawnedActor->SetActorLocation(IntersectionPoint);
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Current SpawnedActor Missing."));
	}
}

bool UTGCustomizingComponent::IsWeaponNearBone()
{
	if (!MySkeletalMeshComponent && CurrentSpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid state for weapon-bone proximity check"));
		return false;
	}

	FVector targetLocation = CurrentSpawnedActor->GetActorLocation();
	FVector closestBoneLocation;
	FName closestBoneName = MySkeletalMeshComponent->FindClosestBone(targetLocation, &closestBoneLocation);

	if (closestBoneName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid bone found for proximity check"));
		return false;
	}

	float closestBoneDistance = FVector::Distance(targetLocation, closestBoneLocation);
	return IsWithinSnapDistance(closestBoneDistance, closestBoneLocation, closestBoneName);
}


bool UTGCustomizingComponent::IsWithinSnapDistance(float distance, const FVector& boneLocation, FName boneName)
{
	if (distance < SnapCheckDistance)
	{
		return SnapActor(boneLocation, distance, boneName);
	}
	return false;
}

void UTGCustomizingComponent::UnSnapActor()
{
	CurrentSpawnedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

bool UTGCustomizingComponent::SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName)
{
	//CurrentSpawnedActor->SetActorLocation(ClosestBoneLocation);
	CurrentSpawnedActor->AttachToComponent(MySkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
	CurrentTargetBone = ClosestBoneName;
	UE_LOG(LogTemp, Log, TEXT("Found Spot! Bone :%s"), *ClosestBoneName.ToString());
	return true;
}



void UTGCustomizingComponent::SaveRotationData() const
{
	if (CurrentRotationSelectedActor->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()))
	{
		const FName TargetID = ITGWeaponInterface::Execute_GetBoneID(CurrentRotationSelectedActor);
		FAttachedActorData FoundActorData;
		if (MyGameInstance->GetWeaponActorData(TargetID, FoundActorData))
		{
			FoundActorData.Rotation = CurrentRotationSelectedActor->GetActorRotation();
			MyGameInstance->SetWeaponActorData(TargetID, FoundActorData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Actor data not found for TargetID: %s"), *TargetID.ToString());
		}
	}
	else if (CurrentRotationSelectedActor->GetClass()->ImplementsInterface(UTGArmourInterface::StaticClass()))
	{
		const FName TargetID = ITGArmourInterface::Execute_GetBoneID(CurrentRotationSelectedActor);
		FAttachedActorData FoundActorData;
		if (MyGameInstance->GetArmourActorData(TargetID, FoundActorData))
		{
			FoundActorData.Rotation = CurrentRotationSelectedActor->GetActorRotation();
			MyGameInstance->SetArmourActorData(TargetID, FoundActorData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Actor data not found for TargetID: %s"), *TargetID.ToString());
		}
	}
}

void UTGCustomizingComponent::ResetHoldingData()
{
	if (CurrentSpawnedActor)
	{
		CurrentSpawnedActor->Destroy();
		CurrentTargetBone = FName();
		CurrentSpawnedActor = nullptr;
		CurrentRotationSelectedActor = nullptr;
	}
}

void UTGCustomizingComponent::SetTargetActorRotation(FQuat Rotation) const
{
	if(CurrentRotationSelectedActor)
	{
		CurrentRotationSelectedActor->AddActorLocalRotation(Rotation, false, nullptr, ETeleportType::None);
	}
}

bool UTGCustomizingComponent::SetCurrentRotationSelectedActor(AActor* TargetActor)
{
	if(TargetActor)
	{
		CurrentRotationSelectedActor = TargetActor;
		return true;
	} else
	{
		return false;
	}
}

void UTGCustomizingComponent::DrawDebugHighlight() const
{
	if (CurrentRotationSelectedActor)
	{
		FVector ActorLocation = CurrentRotationSelectedActor->GetActorLocation();
		float SphereRadius = 2.0f; 
		FColor SphereColor = FColor::Red; 
		DrawDebugSphere(GetWorld(), ActorLocation, SphereRadius, 12, SphereColor, false, -1.0f, 0, 2.0f);
	}
}



