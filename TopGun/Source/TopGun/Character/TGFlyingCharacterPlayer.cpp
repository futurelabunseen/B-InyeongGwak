
#include "TGFlyingCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"

ATGFlyingCharacterPlayer::ATGFlyingCharacterPlayer()
{
    PrimaryActorTick.bCanEverTick = true;
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("CPROFILE_TGCAPSULE"));
    GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
    
    DesiredFOV = 90.0f;
    DefaultSocketOffset = FVector(0.0f, 0.0f, 0.0f);
    DesiredSocketOffset = DefaultSocketOffset;
    FlightRotationInterpSpeed = 5.0f;
    bIsFlying = false;
    bBoost = false;
}

void ATGFlyingCharacterPlayer::BeginPlay()
{
    Super::BeginPlay();
    SetCharacterControl();
    FlyingRotation = GetActorRotation();
}

void ATGFlyingCharacterPlayer::SetCharacterControl() const
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

void ATGFlyingCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATGFlyingCharacterPlayer::Move);
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATGFlyingCharacterPlayer::Look);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ATGFlyingCharacterPlayer::Jump);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATGFlyingCharacterPlayer::StopJumping);
    EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &ATGFlyingCharacterPlayer::Boost);
    EnhancedInputComponent->BindAction(SwitchSceneAction, ETriggerEvent::Triggered, this, &ATGFlyingCharacterPlayer::SwitchScene);
    EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &ATGFlyingCharacterPlayer::Boost);

}


void ATGFlyingCharacterPlayer::EnableFlight()
{
    GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    DesiredFOV = 120.0f;
    DesiredSocketOffset = FVector(150.0f, 0.0f, 90.0f);
    bIsFlying = true;
    
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->BrakingDecelerationFlying = 6000.0f;
    GetCharacterMovement()->MaxAcceleration = 6000.0f;
    GetCharacterMovement()->MaxFlySpeed = 800.0f;
    
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        PC->ClientStartCameraShake(LoadClass<UCameraShakeBase>(nullptr, TEXT("/Game/Flying/CameraShakes/CS_Flight_Loop.CS_Flight_Loop_C")), 0.1f);
        PC->ClientStartCameraShake(LoadClass<UCameraShakeBase>(nullptr, TEXT("/Game/Flying/CameraShakes/CS_Flight_Start.CS_Flight_Start_C")), 0.5f);
    }
}

void ATGFlyingCharacterPlayer::DisableFlight()
{
    GetCharacterMovement()->SetMovementMode(MOVE_Falling);
    DesiredFOV = 90.0f;
    DesiredSocketOffset = DefaultSocketOffset;
    bIsFlying = false;
    
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->BrakingDecelerationFlying = 0.0f;
    GetCharacterMovement()->MaxAcceleration = 1500.0f;
    GetCharacterMovement()->MaxFlySpeed = 800.0f;
    
    bBoost = false;
    
    // Stop camera shake
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        PC->ClientStopCameraShake(LoadClass<UCameraShakeBase>(nullptr, TEXT("/Game/Flying/CameraShakes/CS_Flight_Loop.CS_Flight_Loop_C")));
    }
}

void ATGFlyingCharacterPlayer::UpdateCameraAndArm(float DeltaTime)
{
    if (FollowCamera && CameraBoom)
    {
        // Update FOV
        float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, DesiredFOV, DeltaTime, 3.0f);
        FollowCamera->FieldOfView = NewFOV;

        // Update Socket Offset
        FVector NewSocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, DesiredSocketOffset, DeltaTime, 3.0f);
        CameraBoom->SocketOffset = NewSocketOffset;
    }
}

void ATGFlyingCharacterPlayer::UpdateRotation(float DeltaTime)
{
    FRotator TargetRotation = CalculateTargetRotation();
    SmoothRotation(TargetRotation, 800.0f, FlightRotationInterpSpeed, DeltaTime);
}

void ATGFlyingCharacterPlayer::SmoothRotation(const FRotator& Target, float ConstantSpeed, float SmoothSpeed, float DeltaTime)
{
    FRotator CurrentRotation = GetActorRotation();
    FRotator InterpConstant = FMath::RInterpConstantTo(FlyingRotation, Target, DeltaTime, ConstantSpeed);
    FRotator InterpSmooth = FMath::RInterpTo(CurrentRotation, Target, DeltaTime, SmoothSpeed);
    FlyingRotation = InterpConstant;
    SetActorRotation(InterpSmooth);
}


FRotator ATGFlyingCharacterPlayer::CalculateTargetRotation() const
{
    if (ShouldUseLastVelocityRotation())
    {
        return LastVelocityRotation;
    }
    else
    {
        return GetControlRotation();
    }
}

bool ATGFlyingCharacterPlayer::ShouldUseLastVelocityRotation() const
{
    FVector Velocity = GetVelocity();
    return (Velocity.Size() > 1.0f) && !bBoost;
}

void ATGFlyingCharacterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCameraAndArm(DeltaTime);

    if (bIsFlying)
    {
        UpdateRotation(DeltaTime);
    }

    // Update LastVelocityRotation
    FVector Velocity = GetVelocity();
    if (!Velocity.IsNearlyZero())
    {
        LastVelocityRotation = Velocity.Rotation();
    }
}



void ATGFlyingCharacterPlayer::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    if (bIsFlying)
    {
        const FVector ForwardDirection = Rotation.Vector();
        const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
        FVector MovementDirection = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
        MovementDirection.Normalize();
        
        AddMovementInput(MovementDirection, 1.5f);
    }
    else
    {
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ATGFlyingCharacterPlayer::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void ATGFlyingCharacterPlayer::Jump()
{
    if (bIsFlying)
    {
        DisableFlight();
    }
    else
    {
        EnableFlight();
    }
}

void ATGFlyingCharacterPlayer::StopJumping()
{
    Super::StopJumping();
}

void ATGFlyingCharacterPlayer::Boost(const FInputActionValue& Value)
{
    bBoost = Value.Get<bool>();

    if (bBoost)
    {
        DesiredFOV = 120.0f;
        GetCharacterMovement()->MaxFlySpeed = 3000.0f;
        GetCharacterMovement()->BrakingDecelerationFlying = 6000.0f;
        GetCharacterMovement()->MaxAcceleration = 6000.0f;
    }
    else
    {
        DesiredFOV = 90.0f;
        GetCharacterMovement()->MaxFlySpeed = 800.0f;
        GetCharacterMovement()->BrakingDecelerationFlying = 0.0f;
        GetCharacterMovement()->MaxAcceleration = 1500.0f;
    }
}

void ATGFlyingCharacterPlayer::SwitchScene()
{
    // Implement your scene switching logic here
}
