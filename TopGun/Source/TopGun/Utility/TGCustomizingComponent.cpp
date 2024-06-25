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

void UTGCustomizingComponent::AddButtonToPanel(UScrollBox* TargetPanel, TSubclassOf<UUserWidget> TargetButtonWidget, const FName ID) const
{
	if (!GetOwner() || !TargetPanel || !TargetButtonWidget) return;
	
	UWorld* World = GetOwner()->GetWorld();
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, TargetButtonWidget);
	if (!CreatedWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Created widget null"));
		return;
	}
	if (UTGInventoryWeaponButton* InventoryButton = Cast<UTGInventoryWeaponButton>(CreatedWidget))
	{
		InventoryButton->SetupButton(ID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryWeaponButton."));
		return;
	}
	TargetPanel->AddChild(CreatedWidget);
}

void UTGCustomizingComponent::AddWeaponButtonToPanel(UScrollBox* TargetPanel) const
{
	if (!TargetPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddWeaponButtonToPanel: TargetPanel is null."));
		return;
	}

	if (!WeaponButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddWeaponButtonToPanel: WeaponButtonWidgetClass is null."));
		return;
	}

	if (!WeaponDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddWeaponButtonToPanel: WeaponDataAsset is null."));
		return;
	}


	for (const TPair<FName, UBlueprintGeneratedClass*>& WeaponPair : WeaponDataAsset->BaseWeaponClasses)
	{
		AddButtonToPanel(TargetPanel, WeaponButtonWidgetClass, WeaponPair.Key);
	}
}

void UTGCustomizingComponent::AddModuleButtonToPanel(UScrollBox* TargetPanel) const
{
	if (!TargetPanel || !ModuleButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters. Module"));
		return;
	}

	for (const TPair<FName, FMeshCategoryData>& TargetMap : ModuleDataAsset->BaseMeshComponent)
	{
		AddButtonToPanel(TargetPanel, ModuleButtonWidgetClass, TargetMap.Key);
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


bool UTGCustomizingComponent::AttachWeapon() const
{
	if (!MySkeletalMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkeletalMeshComponent is not valid."));
		return false;
	}

	if (!CurrentSpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentSpawnedActor is not valid or not spawned."));
		return false;
	}

	if (!WeaponDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponDataAsset is null."));
		return false;
	}
	AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(CurrentSpawnedActor->GetClass(), MySkeletalMeshComponent->GetComponentLocation(), FRotator::ZeroRotator);
	ClonedActor->AttachToComponent(MySkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
	ClonedActor->SetActorEnableCollision(true);
	if (ClonedActor->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()) && CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()))
	{
		const FName TempWeaponID = ITGWeaponInterface::Execute_GetWeaponID(CurrentSpawnedActor);
		ITGWeaponInterface::Execute_SetWeaponID(ClonedActor, TempWeaponID);
		ITGWeaponInterface::Execute_SetBoneID(ClonedActor, CurrentTargetBone);
		const FAttachedActorData TempData(TempWeaponID, ClonedActor->GetActorRotation());
		MyGameInstance->AttachedActorsMap.Add(CurrentTargetBone, TempData);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawned actor does not implement ITGWeaponInterface."));
	}
	
	if (!ClonedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn cloned actor."));
		return false;
	}
	
	return true;
}

void UTGCustomizingComponent::RemoveWeaponFromCharacter(AActor* WeaponToRemove) const
{
	if (WeaponToRemove)
	{
		if (WeaponToRemove->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()))
		{
			FName TempBoneID = ITGWeaponInterface::Execute_GetBoneID(WeaponToRemove);
			MyGameInstance->AttachedActorsMap.FindAndRemoveChecked(TempBoneID);
		}

		WeaponToRemove->Destroy();
		WeaponToRemove = nullptr;
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
	if (MySkeletalMeshComponent && CurrentSpawnedActor)
	{
		FVector TargetLocation = CurrentSpawnedActor->GetActorLocation();
		FVector ClosestBoneLocation;
		FName ClosestBoneName = MySkeletalMeshComponent->FindClosestBone(TargetLocation, &ClosestBoneLocation);
		if (!ClosestBoneName.IsNone())
		{
			float ClosestBoneDistance = FVector::Dist(TargetLocation, ClosestBoneLocation);
			if (ClosestBoneDistance < SnapCheckDistance)
			{
				SnapActor(ClosestBoneLocation, ClosestBoneDistance, ClosestBoneName);
				return true;
			} else
			{
				return false;
			}
		} else
		{
			return false;
		}
	} else
	{
		return false;
	}
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
		FAttachedActorData* FoundActorData = MyGameInstance->AttachedActorsMap.Find(TargetID);
		if (FoundActorData)
		{
			FoundActorData->Rotation = CurrentRotationSelectedActor->GetActorRotation();
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

void UTGCustomizingComponent::SetWeaponRotation(FQuat Rotation) const
{
	if(CurrentRotationSelectedActor)
	{
		CurrentRotationSelectedActor->AddActorLocalRotation(Rotation, false, nullptr, ETeleportType::None);
	}
}

void UTGCustomizingComponent::SetCurrentRotationSelectedActor(AActor* TargetActor)
{
	CurrentRotationSelectedActor = TargetActor;
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



