#include "Character/TGCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TGPlayerControlData.h"
#include "Components/CapsuleComponent.h"
#include "GameInstance/TGGameInstance.h"
#include "Interface/TGArmourInterface.h"
#include "Interface/TGWeaponInterface.h"
#include "UI/TGGameOverWidget.h"
#include "UI/TGWaveBarWidget.h"
#include "Utility/TGCharacterStatComponent.h"
#include "Utility/TGEquipmentManager.h"
#include "Utility/TGFlyingComponent.h"


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

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("CPROFILE_TGCAPSULE"));
    GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

    FlyingComponent = CreateDefaultSubobject<UTGFlyingComponent>(TEXT("FlyingComponent"));
    
    KnockBackAmount = 20;

    bIsTransitioning = false;

}


void ATGCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();
    SetCharacterControl();
    
    MyGameInstance = Cast<UTGCGameInstance>(GetGameInstance());
    if (!MyGameInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast GameInstance to UTGCGameInstance."));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }
    int32 TotalDefense = 0;
    MyGameInstance->GetEquipmentManager()->CalculateStatsForEquip(TotalDefense, ETGEquipmentCategory::Armour);
    UE_LOG(LogTemp, Error, TEXT("ATGCharacterPlayer::BeginPlay HP : %d"), TotalDefense);
    Stat->SetMaxHp(TotalDefense);

    MyGameMode = Cast<ATGGameMode>(GetWorld()->GetAuthGameMode());
    if (!MyGameMode.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast GameInstance to UTGCGameInstance."));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }

    
    OriginalCameraOffset =  OriginalCameraOffset = FollowCamera->GetRelativeLocation();
    TargetCameraOffset = OriginalCameraOffset + FVector(0.0f, 0.0f, 100.0f);
    SetupPlayerModel(GetMesh());

   // HpBar->SetVisibility(false);
}

void ATGCharacterPlayer::SetCharacterControl() const
{
    APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        UInputMappingContext* NewMappingContext = DefaultMappingContext;
        if (NewMappingContext)
        {
            Subsystem->AddMappingContext(NewMappingContext, 0);
        }
    }
}

void ATGCharacterPlayer::SetupPlayerModel(USkeletalMeshComponent* TargetMesh)
{
    SetupMesh(TargetMesh);
    AttachEquip(TargetMesh);
    //AttachWeapons(TargetMesh);
    //AttachArmors(TargetMesh);
}

void ATGCharacterPlayer::SetupMesh(USkeletalMeshComponent* TargetMesh)
{
    if (!MyGameInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("MyGameInstance is null."));
        return;
    }

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
}


void ATGCharacterPlayer::AttachIndividualActor(USkeletalMeshComponent* TargetMesh, FName BoneID, FName ActorID, UBlueprintGeneratedClass* ActorClass, const FRotator& Rotation)
{
    AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator);
    if (!ClonedActor || !ClonedActor->Implements<UTGBaseEquipmentInterface>())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn actor for ID: %s"), *ActorID.ToString());
        return;
    }
        USpringArmComponent* ActorSpringArm = NewObject<USpringArmComponent>(this, USpringArmComponent::StaticClass());
    ActorSpringArm->SetupAttachment(TargetMesh, BoneID);
    ActorSpringArm->TargetArmLength = 0.5f;
    ActorSpringArm->bUsePawnControlRotation = false;
    ActorSpringArm->RegisterComponent();

    ClonedActor->AttachToComponent(ActorSpringArm, FAttachmentTransformRules::SnapToTargetIncludingScale);
    ClonedActor->SetActorRotation(Rotation);
    ClonedActor->SetActorEnableCollision(false);

    ITGBaseEquipmentInterface::Execute_InitializeEquipment(ClonedActor, BoneID, ActorID);
    ITGBaseEquipmentInterface::Execute_SetSpringArmComponent(ClonedActor, ActorSpringArm);

    if(ClonedActor->Implements<UTGWeaponInterface>())
       WeaponMap.Add(BoneID, ClonedActor);
    if(ClonedActor->Implements<UTGArmourInterface>())
        ArmourMap.Add(BoneID, ClonedActor);
}





void ATGCharacterPlayer::AttachEquip(USkeletalMeshComponent* TargetMesh)
{

    for (const auto& Elem : MyGameInstance->GetEquipmentManager()->GetEntireAttachedEquipActorMap())
    {
        UE_LOG(LogTemp, Error, TEXT("AttachEquip processing: %s for bone : %s"), *Elem.Key.ActorID.ToString(),  *Elem.Key.BoneID.ToString());
        UBlueprintGeneratedClass* TempEquipClass = MyGameInstance->GetEquipmentManager()->GetEquipClassByID(Elem.Key.ActorID);
        if (!TempEquipClass)
        {
            UE_LOG(LogTemp, Error, TEXT("ArmourClass not found for ArmourID: %s"), *Elem.Key.ActorID.ToString());
            continue;
        }

        AttachIndividualActor(TargetMesh, Elem.Key.BoneID, Elem.Key.ActorID, TempEquipClass, Elem.Value.Rotation);
    }
    
}



void ATGCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATGCharacterPlayer::Jump);
    EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ATGCharacterPlayer::Move);
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATGCharacterPlayer::Look);
    EnhancedInputComponent->BindAction(SwitchSceneAction, ETriggerEvent::Completed, this, &ATGCharacterPlayer::SwitchScene);
    EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ATGCharacterPlayer::AttackStart);
    EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ATGCharacterPlayer::AttackEnd);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATGCharacterPlayer::AimCameraStart);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATGCharacterPlayer::AimCameraEnd);
    EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &ATGCharacterPlayer::Boost);
}

void ATGCharacterPlayer::Move(const FInputActionValue& Value)
{
    if (FlyingComponent)
    {
        FlyingComponent->Move(Value);
    }
}

void ATGCharacterPlayer::Look(const FInputActionValue& Value)
{
    if (FlyingComponent)
    {
        FlyingComponent->Look(Value);
    }
}

void ATGCharacterPlayer::Jump()
{
    if (FlyingComponent&&!GetCharacterMovement()->IsMovingOnGround())
    {
        FlyingComponent->ToggleFlight();
    }else
    {
        ACharacter::Jump();
    }
}

void ATGCharacterPlayer::Boost(const FInputActionValue& Value)
{
    if (FlyingComponent)
    {
        FlyingComponent->Boost(Value);
    }
}

void ATGCharacterPlayer::SwitchScene()
{
    if (bIsTransitioning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Level transition already in progress. Ignoring SwitchScene call."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Starting level transition in SwitchScene."));
    bIsTransitioning = true;
    PrepareForLevelTransition();
    GetWorldTimerManager().ClearAllTimersForObject(this);
    UE_LOG(LogTemp, Log, TEXT("Cleared all timers for character in SwitchScene."));
    if (MyGameInstance.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Changing level to 'Customizing' in SwitchScene."));
        MyGameInstance->ChangeLevel(FName(TEXT("Customizing")));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get TGCGameInstance in SwitchScene"));
        bIsTransitioning = false;
    }
}



void ATGCharacterPlayer::PrepareForLevelTransition()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->DisableInput(PC);
    }

    AttackEnd();
    SetActorEnableCollision(false);
    SetActorHiddenInGame(true);
    GetWorldTimerManager().ClearAllTimersForObject(this);
    bIsAiming = false;
    isDead = false;

    for (auto& Elem : WeaponMap)
    {
        if (Elem.Value)
        {
            Elem.Value->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        }
    }
    for (auto& Elem : ArmourMap)
    {
        if (Elem.Value)
        {
            Elem.Value->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        }
    }

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = 0;
        CameraBoom->SetRelativeLocation(FVector::ZeroVector);
    }
    if (FollowCamera)
    {
        FollowCamera->SetRelativeLocation(FVector::ZeroVector);
    }

    WeaponMap.Empty();
    ArmourMap.Empty();
    SocketActors.Empty();
}

void ATGCharacterPlayer::AttackStart()
{
    if(!isDead)
    {
        AttackCall(true);
    }
}

void ATGCharacterPlayer::AttackEnd()
{
    if(!isDead)
    {
        AttackCall(false);
    }
}

void ATGCharacterPlayer::AttackCall(bool isFiring)
{
    UE_LOG(LogTemp, Log, TEXT("Calling FireWeapon"));
    if(WeaponMap.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Weapon Pool Empty"));
    }
    
    for (const auto& Elem : WeaponMap)
    {
        UE_LOG(LogTemp, Log, TEXT("ITerating"));
        if (AActor* WeaponActor = Elem.Value)
        {
            if (WeaponActor->GetClass()->ImplementsInterface(UTGWeaponInterface::StaticClass()))
            {
                UE_LOG(LogTemp, Log, TEXT("Calling FireWeapon on %s"), *WeaponActor->GetName());
                ITGWeaponInterface::Execute_FunctionFireWeapon(WeaponActor, isFiring, bIsAiming, true, FollowCamera);
                UE_LOG(LogTemp, Log, TEXT("Weapon fired successfully."));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Weapon does not implement the IWeaponInterface."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid weapon actor."));
        }
    }
}




void ATGCharacterPlayer::AimCameraStart()
{
    bIsAiming = true;
}

void ATGCharacterPlayer::AimCameraEnd()
{
   bIsAiming = false;
}

void ATGCharacterPlayer::SetWeaponRotations()
{
    for (auto& Elem : WeaponMap)
    {
        if (AActor* WeaponActor = Elem.Value)
        {
            if(WeaponActor->Implements<UTGWeaponInterface>() && WeaponActor->Implements<UTGBaseEquipmentInterface>())
            {
                USpringArmComponent* WeaponSpringArm = ITGBaseEquipmentInterface::Execute_GetSpringArmComponent(WeaponActor);
                FQuat TargetRotation = ITGWeaponInterface::Execute_GetAimingRotation(WeaponActor, GetScreenAimingPointVector());
                WeaponActor->SetActorRotation(TargetRotation);
            } else
            {
                UE_LOG(LogTemp, Error, TEXT("Weapon Does not Implement Interface: %s"), *Elem.Key.ToString());
            }
        }
    }
}

void ATGCharacterPlayer::ResetWeaponRotations()
{
    for (const auto& Elem : WeaponMap)
    {
        if (AActor* WeaponActor = Elem.Value)
        {
            if(WeaponActor->Implements<UTGWeaponInterface>())
            {
                 USpringArmComponent* WeaponSpringArm = ITGWeaponInterface::Execute_GetSpringArmComponent(WeaponActor);
                 FQuat DefaultQuatRotation = ITGWeaponInterface::Execute_GetDefaultRoationQuat(WeaponActor);
                WeaponActor->SetActorRotation(DefaultQuatRotation);

            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid weapon actor."));
        }
    }
}



void ATGCharacterPlayer::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    FVector CurrentOffset = FollowCamera->GetRelativeLocation();
    FVector DesiredOffset = bIsAiming ? TargetCameraOffset : OriginalCameraOffset;

    if (!CurrentOffset.Equals(DesiredOffset, 1.0f)) 
    {
        FVector NewOffset = FMath::VInterpTo(CurrentOffset, DesiredOffset, DeltaSeconds, InterpSpeed);
        FollowCamera->SetRelativeLocation(NewOffset);
    }
    if(bIsAiming)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        SetActorRotation(YawRotation);
        SetWeaponRotations();
    } else
    {
        ResetWeaponRotations();
    }
}



FVector ATGCharacterPlayer::GetScreenAimingPointVector() const
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController is null."));
        return FVector::ZeroVector;
    }

    int32 ViewportSizeX, ViewportSizeY;
    PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
    FVector2D ScreenCenter(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);

    FVector WorldLocation;
    FVector WorldDirection;
    PlayerController->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection);

    FVector TargetPoint = WorldLocation + (WorldDirection * 10000.0f);

    if (FollowCamera)
    {
        FVector CameraLocation = FollowCamera->GetComponentLocation();
    }

    return TargetPoint;
}


void ATGCharacterPlayer::SetCharacterControlData(const UTGPlayerControlData* CharacterControlData)
{
    Super::SetCharacterControlData(CharacterControlData);
}

void ATGCharacterPlayer::Die()
{
    Super::Die();
    GetMesh()->SetHiddenInGame(true);
    CameraBoom->bDoCollisionTest = false;
    APlayerController* PC = Cast<APlayerController>(GetController());
    if(PC)
    {
        PC->bShowMouseCursor = false;
        isDead = true;
    }
    PrepareForLevelTransition();
}

float ATGCharacterPlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
    AActor* DamageCauser)
{
        return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ATGCharacterPlayer::SetupCharacterWidget(UTGUserWidget* InUserWidget)
{
    Super::SetupCharacterWidget(InUserWidget);

    UTGWaveBarWidget* WaveWidget = Cast<UTGWaveBarWidget>(InUserWidget);
    if (WaveWidget)
    {
        MyGameMode->OnWaveChanged.AddUObject(WaveWidget, &UTGWaveBarWidget::UpdateWaveBar);
    }
}

void ATGCharacterPlayer::ResetInvincibility()
{
    bIsInvincible = false;
}


void ATGCharacterPlayer::SetUpGameOverWidget(UTGUserWidget* InUserWidget)
{
    UTGGameOverWidget* GameOverWidget = Cast<UTGGameOverWidget>(InUserWidget);
    if (GameOverWidget)
    {
        Stat->OnZeroScore.AddUObject(GameOverWidget, &UTGGameOverWidget::UpdateScore);
    }
}


void ATGCharacterPlayer::NotifyActorBeginOverlap(AActor* OtherActor)
{
    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Notify Actor Begin Overlap... Other Actor Name: %s"), *OtherActor->GetName()));

    UE_LOG(LogTemp, Error, TEXT("COLLISION"));
}

void ATGCharacterPlayer::NotifyActorEndOverlap(AActor* OtherActor)
{
    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Notify Actor End Overlap... Other Actor Name: %s"), *OtherActor->GetName()));
}