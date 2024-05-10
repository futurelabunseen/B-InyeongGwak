// Fill out your copyright notice in the Description page of Project Settings.
#include "Character/TGCustomizingCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SkeletalMeshMerge.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/ScrollBox.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/TGInventoryWeaponButton.h"
#include "Utility/TGModuleDataAsset.h"
#include "Utility/TGWeaponDataAsset.h"
#include "Utility/TGModuleSystem.h"

ATGCustomizingCharacterBase::ATGCustomizingCharacterBase()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CurrentState = ECustomizingState::Idle;
	
	BodyPartIndex.Add(E_PartsCode::Head, FName("HeadA"));
	BodyPartIndex.Add(E_PartsCode::UpperBody, FName("BodyA"));
	BodyPartIndex.Add(E_PartsCode::LowerBody, FName("LegA"));
	BodyPartIndex.Add(E_PartsCode::LeftHand, FName("LeftArmA"));
	BodyPartIndex.Add(E_PartsCode::RightHand, FName("RightArmA"));
}

void ATGCustomizingCharacterBase::BeginPlay()
{
	UE_LOG(LogTemp, Log, TEXT("BeginPlay"));
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = true; 
	PrimaryActorTick.bStartWithTickEnabled = true;

	myPlayerController = CastChecked<APlayerController>(GetController());
	if (myPlayerController)
	{
		myPlayerController->bShowMouseCursor = true; 
		myPlayerController->bEnableClickEvents = true; 
		myPlayerController->bEnableMouseOverEvents = true;
	}
	mySkeletalMeshComponent = GetMesh();
	SetActorEnableCollision(false);
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(myPlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = DefaultMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}
}

void ATGCustomizingCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	for (const auto& Pair : BodyPartIndex)
	{
		const UEnum* EnumPtr = StaticEnum<E_PartsCode>();
		FString KeyName = EnumPtr ? EnumPtr->GetNameStringByValue(static_cast<int64>(Pair.Key)) : TEXT("Unknown Key");
		FString ValueName = Pair.Value.ToString();
		UE_LOG(LogTemp, Log, TEXT("Key: %s, Value: %s"), *KeyName, *ValueName);
	}
	switch (CurrentState)
	{
	case ECustomizingState::Idle:
		break;
	case ECustomizingState::OnDragActor:
		UpdateWeaponActorPosition();
		CheckWeaponActorProximity();
		break;
	case ECustomizingState::OnSnappedActor:
		CheckSnappedCancellation();
		break;
	}
}

void ATGCustomizingCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Triggered, this, &ATGCustomizingCharacterBase::OnClickRightMouse);
	EnhancedInputComponent->BindAction(RClickAction, ETriggerEvent::Triggered, this, &ATGCustomizingCharacterBase::OnClickLeftMouse);
	EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Triggered, this, &ATGCustomizingCharacterBase::OnClickEscape);
	EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ATGCustomizingCharacterBase::OnRotateAction);
}



void ATGCustomizingCharacterBase::OnClickRightMouse()
{
	switch (CurrentState)
	{
	case ECustomizingState::Idle:
		break;
	case ECustomizingState::OnDragActor:
		break;
	case ECustomizingState::OnSnappedActor:
		AttachWeapon();
		break;
	}
}

void ATGCustomizingCharacterBase::OnClickLeftMouse()
{
	UE_LOG(LogTemp, Log, TEXT("LeftMouseClicked State : %d"), CurrentState);
	switch (CurrentState)
	{
	case ECustomizingState::Idle:
		RemoveWeaponInDesiredPosition();
		break;
	case ECustomizingState::OnDragActor:
		ReturnToIdleState();
		break;
	case ECustomizingState::OnSnappedActor:
		ReturnToIdleState();
		break;
	}
}

void ATGCustomizingCharacterBase::RemoveWeaponInDesiredPosition()
{
	if (!myPlayerController) return;
	float MouseX, MouseY;
	if (myPlayerController->GetMousePosition(MouseX, MouseY))
	{
		FVector WorldLocation, WorldDirection;
		if (myPlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			FHitResult Hit;
			FVector Start = WorldLocation;
			FVector End = Start + WorldDirection * 10000;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this); 
			ECollisionChannel Channel = ECC_Visibility;
			if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Channel, QueryParams))
			{
				//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f, 0, 1.0f);
				if (Hit.GetActor())
				{
					ATGBaseWeapon* HitWeapon = Cast<ATGBaseWeapon>(Hit.GetActor());
					if (HitWeapon)
					{
						RemoveWeaponFromCharacter(HitWeapon);
					}
				}
			}
		}
	}
}

void ATGCustomizingCharacterBase::OnClickEscape()
{
	ReturnToIdleState();
}

void ATGCustomizingCharacterBase::OnRotateAction(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();
	AddControllerYawInput(InputVector.Y);
	if (FollowCamera)
	{
		float ZoomFactor = 10.0f; 
		float NewOrthoWidth = FollowCamera->OrthoWidth + InputVector.X * -ZoomFactor;
		float MinOrthoWidth = 100.0f;  
		float MaxOrthoWidth = 1000.0f; 
		FollowCamera->OrthoWidth = FMath::Clamp(NewOrthoWidth, MinOrthoWidth, MaxOrthoWidth);
	} else
	{
		UE_LOG(LogTemp, Log, TEXT("Camera null"));
	}
}

void ATGCustomizingCharacterBase::UpdateWeaponActorPosition() const
{
	if (myPlayerController)
	{
		float MouseX, MouseY;
		if (myPlayerController->GetMousePosition(MouseX, MouseY))
		{
			FVector WorldLocation, WorldDirection;
			if (myPlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
			{
				FVector PlaneNormal = FVector(1, 0, 0);  // Upward facing normal
				FVector PlanePoint = FVector(0, 0, 100); // Plane 100 units up in Z
				FVector FarPoint = WorldLocation + WorldDirection * 10000;
				FVector IntersectionPoint = FMath::LinePlaneIntersection(WorldLocation, FarPoint, PlanePoint, PlaneNormal);
				if (CurrentSpawnedActor.IsValid())
				{
					CurrentSpawnedActor->SetActorLocation(IntersectionPoint);
				}
			}
		}
	}
}



void ATGCustomizingCharacterBase::RemoveWeaponFromCharacter(ATGBaseWeapon* WeaponToRemove)
{
	if (WeaponToRemove)
	{
		WeaponToRemove->Destroy();
		WeaponToRemove = nullptr;	}
	else
	{
		return;
	}
}


void ATGCustomizingCharacterBase::CheckWeaponActorProximity()
{
	if (mySkeletalMeshComponent && CurrentSpawnedActor.IsValid())
	{
		FVector TargetLocation = CurrentSpawnedActor->GetActorLocation();
		FVector ClosestBoneLocation;
		FName ClosestBoneName = mySkeletalMeshComponent->FindClosestBone(
			TargetLocation,
			&ClosestBoneLocation
		);
		if (!ClosestBoneName.IsNone())
		{
			float ClosestBoneDistance = FVector::Dist(TargetLocation, ClosestBoneLocation);
			if(ClosestBoneDistance < 5.0f)
			{
				CurrentState = ECustomizingState::OnSnappedActor;
				CurrentSpawnedActor->SetActorLocation(ClosestBoneLocation);
				CurrentTargetBone = ClosestBoneName;
				UE_LOG(LogTemp, Log, TEXT("Found Spot! Bone :%s"), *ClosestBoneName.ToString());
			}
		}
	} else
		return;
}

void ATGCustomizingCharacterBase::CheckSnappedCancellation()
{
	if (myPlayerController)
	{
		float MouseX, MouseY;
		if (myPlayerController->GetMousePosition(MouseX, MouseY))
		{
			FVector WorldLocation, WorldDirection;
			if (myPlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
			{
				if(FVector::Distance(CurrentSpawnedActor->GetActorLocation(), WorldLocation) > 10)
				{
					CurrentState = ECustomizingState::OnDragActor;
				}
			}
		}
	}
}


void ATGCustomizingCharacterBase::AttachWeapon()
{
	if (!mySkeletalMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkeletalMeshComponent is not valid."));
		return;
	}

    if (!CurrentSpawnedActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentSpawnedActor is not valid or not spawned."));
		return;
	}

	if (CurrentTargetBone.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentTargetBone is not specified."));
		return;
	}
	AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(CurrentSpawnedActor->GetClass(), mySkeletalMeshComponent->GetComponentLocation(), FRotator::ZeroRotator);
	ClonedActor->AttachToComponent(mySkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
	ClonedActor->SetActorEnableCollision(true);

	if (!ClonedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn cloned actor."));
		return;
	}
	ReturnToIdleState();
}




void ATGCustomizingCharacterBase::ReturnToIdleState()
{
    if (CurrentSpawnedActor.IsValid())
	{
		CurrentSpawnedActor->Destroy();
		CurrentSpawnedActor = nullptr; // Also, nullify the pointer after destroying
	}
	CurrentState = ECustomizingState::Idle;

}


void ATGCustomizingCharacterBase::AddButtonToPanel(UScrollBox* TargetPanel)
{
	if (!TargetPanel ||!ButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters. WEapon"));
		return;
	}

	UTGWeaponDataAsset* CastedWeaponDataAsset = Cast<UTGWeaponDataAsset>(WeaponDataAsset);
	
	for (const TPair<FName, UBlueprintGeneratedClass*>& WeaponPair : CastedWeaponDataAsset->BaseWeaponClasses)
	{
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(GetWorld(), ButtonWidgetClass);
		if (!CreatedWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create widget for weapon: %s"), *WeaponPair.Key.ToString());
			continue;
		}

		UTGInventoryWeaponButton* InventoryButton = Cast<UTGInventoryWeaponButton>(CreatedWidget);
		if (InventoryButton)
		{
			InventoryButton->SetupButton(WeaponPair.Key);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryWeaponButton."));
			continue;
		}
		
		TargetPanel->AddChild(CreatedWidget);
	}
}

void ATGCustomizingCharacterBase::AddModuleButtonToPanel(UScrollBox* TargetPanel)
{
	if (!TargetPanel ||!ModuleButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters. Module"));
		return;
	}

	UTGModuleDataAsset* CastedModuleDataAsset = Cast<UTGModuleDataAsset>(ModuleDataAsset);

	for (const TPair<FName, FMeshCategoryData> targetMap : CastedModuleDataAsset->BaseMeshComponent)
	{
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(GetWorld(), ModuleButtonWidgetClass);
		if (!CreatedWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create widget for module: %s"), *targetMap.Key.ToString());
			continue;
		}
		UTGInventoryWeaponButton* InventoryButton = Cast<UTGInventoryWeaponButton>(CreatedWidget);
		if (InventoryButton)
		{
			InventoryButton->SetupButton(targetMap.Key);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryModuleButton."));
			continue;
		}
		TargetPanel->AddChild(CreatedWidget);
	}
	
}


void ATGCustomizingCharacterBase::OnWeaponSelected(FName WeaponID)
{
	if (CurrentSpawnedActor.IsValid())
	{
		CurrentSpawnedActor->Destroy();
	}
	
	UTGWeaponDataAsset* CastedWeaponDataAsset = Cast<UTGWeaponDataAsset>(WeaponDataAsset);
	
	UBlueprintGeneratedClass** FoundWeaponClass = CastedWeaponDataAsset->BaseWeaponClasses.Find(WeaponID);
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = GetInstigator();

	
	CurrentSpawnedActor = GetWorld()->SpawnActor<AActor>(*FoundWeaponClass, GetActorLocation(), GetActorRotation(), SpawnParameters);
	CurrentSpawnedActor->SetActorEnableCollision(false);
	
    if (!CurrentSpawnedActor.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn weapon."));
		return;
	}
	
	CurrentWeaponClass = *FoundWeaponClass;
	CurrentState = ECustomizingState::OnDragActor;
}

void ATGCustomizingCharacterBase::OnModuleSelected(FName WeaponID)
{
	UTGModuleDataAsset* CastedModuleDataAsset = Cast<UTGModuleDataAsset>(ModuleDataAsset);
	const FMeshCategoryData* TargetData = CastedModuleDataAsset->BaseMeshComponent.Find(WeaponID);
	BodyPartIndex[TargetData->Category] = WeaponID;
	UClass* AnimClass = GetMesh() -> GetAnimClass();
	USkeleton* Skeleton = GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
	USkeletalMesh* MergedMesh = GetMergeCharacterParts(BodyPartIndex, CastedModuleDataAsset);
	if(MergedMesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to merge Mesh."));
		return;
	}
	MergedMesh->USkeletalMesh::SetSkeleton(Skeleton);
	GetMesh()->SetSkeletalMesh(MergedMesh);
	GetMesh()->SetAnimInstanceClass(AnimClass);
}

USkeletalMesh* ATGCustomizingCharacterBase::GetMergeCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TSoftObjectPtr<UTGModuleDataAsset> ModuleAsset)
{
	TArray<USkeletalMesh*> PartsToMerge;
	for (const auto& Elem : WholeModuleData)
	{
		PartsToMerge.Add(ModuleAsset->GetMeshByID(Elem.Value));
	}
	if (PartsToMerge.IsEmpty())
	{
		return nullptr;
	}
	USkeletalMesh* MergedMesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!MergedMesh)
	{
		return nullptr;
	}
	FSkeletalMeshMerge MeshMerger(MergedMesh, PartsToMerge, TArray<FSkelMeshMergeSectionMapping>(), 0);
	if (bool bMergeSuccess = MeshMerger.DoMerge())
	{
		return MergedMesh;
	} else
	{
		return nullptr;
	}
}


