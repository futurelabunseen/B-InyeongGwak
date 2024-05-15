// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TGCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TGPlayerControlData.h"
#include "GameInstance/TGGameInstance.h"
#include "Utility/TGWeaponDataAsset.h"
#include "Weapon/TGBaseWeapon.h"

ATGCharacterPlayer::ATGCharacterPlayer()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	PlayerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomMesh"));
	PlayerMesh->SetupAttachment(RootComponent);

	CurrentCharacterControlType = ECharacterControlType::Walking;
	
	
}

void ATGCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();
	SetCharacterControl(CurrentCharacterControlType);
	MyGameInstance = Cast<UTGCGameInstance>(GetGameInstance());
	if (!MyGameInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast GameInstance to UTGCGameInstance."));
		FGenericPlatformMisc::RequestExit(false);
		return;
	}

	for (const FString& socketName : socketNames)
	{
		ATGBaseWeapon* placeholder = GetWorld()->SpawnActor<ATGBaseWeapon>(ATGBaseWeapon::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (placeholder)
		{
			placeholder->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(*socketName));
		}
	}

	SetupPlayerModel(GetMesh());
}

void ATGCharacterPlayer::SetupPlayerModel(USkeletalMeshComponent* TargetMesh)
{
	UClass* AnimClass = TargetMesh->GetAnimClass();
	USkeleton* Skeleton = TargetMesh->GetSkeletalMeshAsset()->GetSkeleton();
	USkeletalMesh* MergedMesh = UTGModuleSystem::GetMergeCharacterParts(MyGameInstance->ModuleBodyPartIndex, MyGameInstance->ModuleDataAsset);
	if (MergedMesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to merge Mesh."));
		return;
	}
	MergedMesh->USkeletalMesh::SetSkeleton(Skeleton);
	TargetMesh->SetSkeletalMesh(MergedMesh);
	TargetMesh->SetAnimInstanceClass(AnimClass);
	
	for (const auto& Elem: MyGameInstance->AttachedActorsMap)
	{
		FName BoneID = Elem.Key;
		FName WeaponID = Elem.Value.ActorID;
		UBlueprintGeneratedClass* WeaponClass = *MyGameInstance->WeaponDataAsset->BaseWeaponClasses.Find(WeaponID);
		AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator);
		ClonedActor->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneID);
		ClonedActor->SetActorRotation(Elem.Value.Rotation);
		ClonedActor->SetActorEnableCollision(false);

		TWeakObjectPtr<ATGBaseWeapon> WeaponPointer = Cast<ATGBaseWeapon>(ClonedActor);
		if (WeaponPointer.IsValid())
		{
			WeaponMap.Add(BoneID, WeaponPointer); // Save the weapon pointer in the map
		}
	}
}


void ATGCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ATGCharacterPlayer::Walk);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATGCharacterPlayer::Look);
	EnhancedInputComponent->BindAction(FlyAction, ETriggerEvent::Completed, this, &ATGCharacterPlayer::Fly);
	EnhancedInputComponent->BindAction(SwitchSceneAction, ETriggerEvent::Completed, this, &ATGCharacterPlayer::SwitchScene);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ATGCharacterPlayer::Attack);
}

void ATGCharacterPlayer::SwitchScene()
{
	MyGameInstance->ChangeLevel(FName(TEXT("Customizing")));
}

void ATGCharacterPlayer::Attack()
{
	for(const auto& Elem: WeaponMap)
	{
		UE_LOG(LogTemp, Log, TEXT("Attack Iter"));
		if(ITGWeaponInterface* WeaponInterface = Cast<ITGWeaponInterface>(Elem.Value))
		{
			UE_LOG(LogTemp, Log, TEXT("AttackCasted"));
			WeaponInterface->FunctionFireWeapon(false, false, CameraBoom);
		}
	}
}


void ATGCharacterPlayer::Walk(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ATGCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Log, TEXT("LookAxisVector : X = %f, Y = %f"), LookAxisVector.X, LookAxisVector.Y);

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}

void ATGCharacterPlayer::Fly(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ATGCharacterPlayer::ChangeCharacterControl()
{
	if (CurrentCharacterControlType == ECharacterControlType::Flying)
	{
		SetCharacterControl(ECharacterControlType::Walking);
		APlayerController* PC = Cast<APlayerController>(GetController());
		PC->bShowMouseCursor = false;
	}
	else if (CurrentCharacterControlType == ECharacterControlType::Walking)
	{
		SetCharacterControl(ECharacterControlType::Flying);
		APlayerController* PC = Cast<APlayerController>(GetController());
		PC->bShowMouseCursor = true;
	}
}

void ATGCharacterPlayer::SetCharacterControl(ECharacterControlType NewCharacterControlType)
{
	UTGPlayerControlData* NewCharacterControl = CharacterControlManager[NewCharacterControlType];
	check(NewCharacterControl);

	SetCharacterControlData(NewCharacterControl);

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = NewCharacterControl->InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}

	CurrentCharacterControlType = NewCharacterControlType;
}

void ATGCharacterPlayer::SetCharacterControlData(const UTGPlayerControlData* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);
}

void ATGCharacterPlayer::WalkingMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ATGCharacterPlayer::WalkingLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ATGCharacterPlayer::FlyingMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ATGCharacterPlayer::FlyingLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}
