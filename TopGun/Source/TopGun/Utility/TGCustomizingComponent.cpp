#include "TGCustomizingComponent.h"

#include "TGEquipmentManager.h"
#include "Blueprint/UserWidget.h"
#include "Character/TGCustomizingCharacterBase.h"
#include "Components/ScrollBox.h"
#include "UI/TGInventoryWeaponButton.h"
#include "Utility/TGModuleSystem.h"
#include "Utility/TGModuleDataAsset.h"
#include "GameFramework/Actor.h"
#include "GameInstance/TGGameInstance.h"
#include "Interface/TGBaseEquipmentInterface.h"
#include "UI/TGEquipWidget.h"


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
}

USkeletalMesh* UTGCustomizingComponent::GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset)
{
	return UTGModuleSystem::GetMergeCharacterParts(WholeModuleData, ModuleDataAsset.Get());
}


void UTGCustomizingComponent::GenerateEquipButtonProcessEquipRow(const FName& Key, UScrollBox* TargetPanel) const
{
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(GetWorld(), WeaponButtonWidgetClass);
	if (!CreatedWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create widget"));
		return;
	}

	if (auto* InventoryButton = Cast<UTGEquipWidget>(CreatedWidget))
	{
		InventoryButton->SetupButton(Key, MyGameInstance->GetEquipmentManager()->GetEquipPointsByID(Key));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryEquipButton"));
		return;
	}

	TargetPanel->AddChild(CreatedWidget);
}



void UTGCustomizingComponent::GenerateEquipButtons(UScrollBox* TargetPanel, ETGEquipmentCategory category) const
{
	if (!TargetPanel || !WeaponButtonWidgetClass || !ArmourButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters for GenerateEquipButtons"));
		return;
	}

	TArray<TPair<FName, FEquipmentData>> EquipmentDataArray = MyGameInstance->GetEquipmentManager()->GetEquipmentDataForCategory(category);

	for (const auto& Pair : EquipmentDataArray)
	{
		GenerateEquipButtonProcessEquipRow(Pair.Key, TargetPanel);
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

AActor* UTGCustomizingComponent::SpawnEquip(FName EquipID)
{
	UBlueprintGeneratedClass* EquipmentClass = MyGameInstance->GetEquipmentManager()->GetEquipClassByID(EquipID);
	if (!EquipmentClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UTGCustomizingComponent::SpawnEquip/EquipmentClass not found."));
		return nullptr;
	}
	FActorSpawnParameters SpawnParameters;

	return GetWorld()->SpawnActor<AActor>(EquipmentClass);
}



void UTGCustomizingComponent::SpawnCurrentEquip(FName EquipID)
{
	CurrentSpawnedActor = SpawnEquip(EquipID);
	if (!CurrentSpawnedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("UTGCustomizingComponent::SpawnCurrentEquip/Failed to spawn Equipment."));
		return;
	}
	
	CurrentSpawnedActor->SetActorEnableCollision(false);
	if (CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnCurrentEquip::Interface Called -> EquipID: %s"), *EquipID.ToString());
		ITGBaseEquipmentInterface::Execute_SetEquipmentID(CurrentSpawnedActor, EquipID);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UTGCustomizingComponent::SpawnCurrentEquip/Spawned actor does not implement ITGEquipInterface."));
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
	if(!EquipRegister(ClonedActor))
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

bool UTGCustomizingComponent::EquipRegister(AActor* ClonedActor) const
{
	if (ClonedActor->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()) && CurrentSpawnedActor->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()))
	{
		const FName TempEquipID = ITGBaseEquipmentInterface::Execute_GetEquipmentID(CurrentSpawnedActor);
		UE_LOG(LogTemp, Warning, TEXT("Registering EquipID: %s"), *TempEquipID.ToString());
		const ETGEquipmentCategory TempEquipCategory = ITGBaseEquipmentInterface::Execute_GetCategory(CurrentSpawnedActor);
		ITGBaseEquipmentInterface::Execute_SetEquipmentID(ClonedActor, TempEquipID);
		ITGBaseEquipmentInterface::Execute_SetBoneID(ClonedActor, CurrentTargetBone);
		const FAttachedActorData TempData(TempEquipID, ClonedActor->GetActorRotation());
		FEquipmentKey TargetKey (CurrentTargetBone, TempEquipCategory,TempEquipID);
		MyGameInstance->GetEquipmentManager()->SetEquipActorData(TargetKey, TempData);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawned actor does not implement ITGWeaponInterface."));
		return false;
	}
}



void UTGCustomizingComponent::RemoveEquipFromCharacter(AActor* EquipToRemove) const
{
	if (EquipToRemove)
	{
		if (EquipToRemove->GetClass()->ImplementsInterface(UTGBaseEquipmentInterface::StaticClass()))
		{
			MyGameInstance->GetEquipmentManager()->RemoveFromEquipActorsMap(MyGameInstance->GetEquipmentManager()->GetKeyForActor(EquipToRemove));
		}

		EquipToRemove->Destroy();
		EquipToRemove = nullptr;
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

bool UTGCustomizingComponent::IsEquipNearBone()
{
	if (!MySkeletalMeshComponent || !CurrentSpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid state for equipment-bone proximity check: MySkeletalMeshComponent: %s, CurrentSpawnedActor: %s"), 
			   MySkeletalMeshComponent ? TEXT("Valid") : TEXT("Invalid"),
			   CurrentSpawnedActor ? TEXT("Valid") : TEXT("Invalid"));
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
	CurrentSpawnedActor->AttachToComponent(MySkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
	CurrentTargetBone = ClosestBoneName;
	UE_LOG(LogTemp, Log, TEXT("Found Spot! Bone :%s"), *ClosestBoneName.ToString());
	return true;
}

void UTGCustomizingComponent::HighlightSelectedActor(bool bEnable)
{
	if (bIsHighlighted == bEnable)
		return;

	bIsHighlighted = bEnable;

	if (!CurrentSpawnedActor)
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

void UTGCustomizingComponent::SaveRotationData() const
{
		const FName TargetID = ITGBaseEquipmentInterface::Execute_GetBoneID(CurrentSelectedActor);
		FAttachedActorData FoundActorData;
		if (MyGameInstance->GetEquipmentManager()->GetEquipActorData(MyGameInstance->GetEquipmentManager()->GetKeyForActor(CurrentSelectedActor), FoundActorData))
		{
			FoundActorData.Rotation = CurrentSelectedActor->GetActorRotation();
			MyGameInstance->GetEquipmentManager()->SetEquipActorData(MyGameInstance->GetEquipmentManager()->GetKeyForActor(CurrentSpawnedActor), FoundActorData);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Actor data not found for TargetID: %s"), *TargetID.ToString());
		}
}

void UTGCustomizingComponent::ResetHoldingData()
{
	if (CurrentSpawnedActor)
	{
		CurrentSpawnedActor->Destroy();
		CurrentTargetBone = FName();
		CurrentSpawnedActor = nullptr;
		CurrentSelectedActor = nullptr;
	}
}

void UTGCustomizingComponent::SetTargetActorRotation(FQuat Rotation) const
{
	if(CurrentSelectedActor)
	{
		CurrentSelectedActor->AddActorLocalRotation(Rotation, false, nullptr, ETeleportType::None);
	}
}

bool UTGCustomizingComponent::SetCurrentSelectedActor(AActor* TargetActor)
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

AActor* UTGCustomizingComponent::GetCurrentSelectedActor() const
{
	if(CurrentSelectedActor)
	{
		return CurrentSelectedActor;
	} else
	{
		return nullptr;
	}
}

void UTGCustomizingComponent::DrawDebugHighlight() const
{
	if (CurrentSelectedActor)
	{
		FVector ActorLocation = CurrentSelectedActor->GetActorLocation();
		float SphereRadius = 2.0f; 
		FColor SphereColor = FColor::Red; 
		DrawDebugSphere(GetWorld(), ActorLocation, SphereRadius, 12, SphereColor, false, -1.0f, 0, 2.0f);
	}
}



