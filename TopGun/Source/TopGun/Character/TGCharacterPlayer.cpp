#include "Character/TGCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "TGPlayerControlData.h"
#include "Components/CapsuleComponent.h"
#include "GameInstance/TGGameInstance.h"
#include "Utility/TGWeaponDataAsset.h"
#include "Interface/TGWeaponInterface.h"
#include "Utility/TGCharacterStatComponent.h"


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
    
    CurrentCharacterControlType = ECharacterControlType::Walking;
    Stat->SetHp(300);
    KnockBackAmount = 20;
}

ATGCharacterPlayer::~ATGCharacterPlayer()
{// Log the destruction for debugging purposes
    UE_LOG(LogTemp, Warning, TEXT("Destructor for ATGCharacterPlayer called."));

    // Ensure the game instance is properly cleaned up
    MyGameInstance.Reset();

    // Clean up any dynamically allocated resources or registered events here
    for (auto& Elem : WeaponMap)
    {
        if (AActor* WeaponActor = Elem.Value)
        {
            WeaponActor->Destroy();
        }
    }

    WeaponMap.Empty();

    // If any other components or resources need explicit cleanup, do it here
    // Example:
    // if (SomeComponent)
    // {
    //     SomeComponent->DestroyComponent();
    //     SomeComponent = nullptr;
    // }
}

void ATGCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();
    MyGameInstance = Cast<UTGCGameInstance>(GetGameInstance());
    if (!MyGameInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast GameInstance to UTGCGameInstance."));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }
/*
    for (const FString& socketName : socketNames)
    {
        ATGBaseWeapon* placeholder = GetWorld()->SpawnActor<ATGBaseWeapon>(ATGBaseWeapon::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
        if (placeholder)
        {
            placeholder->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(*socketName));
        }
    }
*/
    OriginalCameraOffset =  OriginalCameraOffset = FollowCamera->GetRelativeLocation();
    TargetCameraOffset = OriginalCameraOffset + FVector(0.0f, 0.0f, 100.0f);
    SetupPlayerModel(GetMesh());
}

void ATGCharacterPlayer::SetupPlayerModel(USkeletalMeshComponent* TargetMesh)
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

    if(!MyGameInstance->AttachedActorsMap.IsEmpty()){
    for (const auto& Elem : MyGameInstance->AttachedActorsMap)
    {
        if (Elem.Key.IsNone() || Elem.Value.ActorID.IsNone())
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid BoneID or WeaponID."));
            continue;
        }

        FName BoneID = Elem.Key;
        FName WeaponID = Elem.Value.ActorID;

        // Debugging log
        UE_LOG(LogTemp, Log, TEXT("Attempting to find WeaponClass for WeaponID: %s"), *WeaponID.ToString());

        UBlueprintGeneratedClass** WeaponClassPtr = MyGameInstance->WeaponDataAsset->BaseWeaponClasses.Find(WeaponID);
        if (!WeaponClassPtr)
        {
            UE_LOG(LogTemp, Error, TEXT("WeaponClass not found for WeaponID: %s"), *WeaponID.ToString());
            continue;
        }

        UBlueprintGeneratedClass* WeaponClass = *WeaponClassPtr;
        AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator);
        if (ClonedActor)
        {
            USpringArmComponent* WeaponSpringArm = NewObject<USpringArmComponent>(this, USpringArmComponent::StaticClass());
            WeaponSpringArm->SetupAttachment(TargetMesh, BoneID);
            WeaponSpringArm->TargetArmLength = 0.5f;
            WeaponSpringArm->bUsePawnControlRotation = false;
            WeaponSpringArm->RegisterComponent();

            ClonedActor->AttachToComponent(WeaponSpringArm, FAttachmentTransformRules::SnapToTargetIncludingScale);
            ClonedActor->SetActorRotation(Elem.Value.Rotation);
            ClonedActor->SetActorEnableCollision(false);

            if(ITGWeaponInterface* WeaponInterface = Cast<ITGWeaponInterface>(ClonedActor))
            {
                WeaponInterface->InitializeWeapon(BoneID, WeaponID);
                WeaponInterface->SetSpringArmComponent(WeaponSpringArm);
            }
            WeaponMap.Add(BoneID, ClonedActor);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn weapon actor."));
        }
    }
    }
    
}

void ATGCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

   // EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
   // EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
   // EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ATGCharacterPlayer::Walk);
   // EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATGCharacterPlayer::Look);
    EnhancedInputComponent->BindAction(SwitchSceneAction, ETriggerEvent::Completed, this, &ATGCharacterPlayer::SwitchScene);
    EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ATGCharacterPlayer::AttackStart);
    EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ATGCharacterPlayer::AttackEnd);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATGCharacterPlayer::AimCameraStart);
    EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATGCharacterPlayer::AimCameraEnd);
}

void ATGCharacterPlayer::SwitchScene()
{
    MyGameInstance->ChangeLevel(FName(TEXT("Customizing")));
}

void ATGCharacterPlayer::AttackStart()
{
    AttackCall(true);
}

void ATGCharacterPlayer::AttackEnd()
{
    AttackCall(false);
}

void ATGCharacterPlayer::AttackCall(bool isFiring)
{
    for (const auto& Elem : WeaponMap)
    {
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


void ATGCharacterPlayer::Walk(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
}

void ATGCharacterPlayer::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(-LookAxisVector.Y);
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
            if(ITGWeaponInterface* WeaponInterface = Cast<ITGWeaponInterface>(WeaponActor))
            {
                USpringArmComponent* WeaponSpringArm = CastChecked<USpringArmComponent>(WeaponInterface->GetSpringArmComponent());
                FQuat TargetRotation = WeaponInterface->GetAimingRotation(GetScreenAimingPointVector());
                WeaponActor->SetActorRotation(TargetRotation);
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
            if(ITGWeaponInterface* WeaponInterface = Cast<ITGWeaponInterface>(WeaponActor))
            {
                 USpringArmComponent* WeaponSpringArm = WeaponInterface->GetSpringArmComponent();
                 FQuat DefaultQuatRotation = WeaponInterface->GetDefaultRoationQuat();
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
      //  DrawDebugLine(GetWorld(), CameraLocation, TargetPoint, FColor::Green, false, 5.0f, 0, 1.0f);
    }

    return TargetPoint;
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

void ATGCharacterPlayer::Die()
{

    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetAllBodiesSimulatePhysics(true);
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->WakeAllRigidBodies();
    UE_LOG(LogTemp, Warning, TEXT("Dead Ragdoll"));
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

void ATGCharacterPlayer::NotifyActorBeginOverlap(AActor* OtherActor)
{
    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Notify Actor Begin Overlap... Other Actor Name: %s"), *OtherActor->GetName()));

    UE_LOG(LogTemp, Error, TEXT("COLLISION"));
}

void ATGCharacterPlayer::NotifyActorEndOverlap(AActor* OtherActor)
{
    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Notify Actor End Overlap... Other Actor Name: %s"), *OtherActor->GetName()));
}
