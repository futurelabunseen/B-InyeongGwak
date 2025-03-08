#include "TGFlyingComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UTGFlyingComponent::UTGFlyingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    DesiredFOV = 90.0f;
    DefaultSocketOffset = FVector(0.0f, 0.0f, 0.0f);
    DesiredSocketOffset = DefaultSocketOffset;
    FlightRotationInterpSpeed = 5.0f;
    bIsFlying = false;
    bBoost = false;

    GroundBrakingDeceleration = 2000.0f;
    FlyingBrakingDeceleration = 6000.0f;
    NormalFlySpeed = 800.0f;
    BoostFlySpeed = 3000.0f;

    BoostDuration = 0.0f;
    MaxBoostDuration = 2.0f;
    StiffDecelerationMultiplier = 2.0f;
    bIsStiffDecelerating = false;
    bWantsToFly = false;
}

void UTGFlyingComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CharacterMovement = OwnerCharacter->GetCharacterMovement();
        FollowCamera = Cast<UCameraComponent>(OwnerCharacter->GetComponentByClass(UCameraComponent::StaticClass()));
        CameraBoom = Cast<USpringArmComponent>(OwnerCharacter->GetComponentByClass(USpringArmComponent::StaticClass()));
    }

    //FlyingRotation = GetOwner()->GetActorRotation();
}

void UTGFlyingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateCameraAndArm(DeltaTime);
    UpdateGroundDetection();

    if (bIsFlying)
    {
        UpdateRotation(DeltaTime);

        if (bBoost)
        {
            BoostDuration += DeltaTime;
            if (BoostDuration >= MaxBoostDuration && !bIsStiffDecelerating)
            {
                StartStiffDeceleration();
            }
        }
        else
        {
            BoostDuration = 0.0f;
            bIsStiffDecelerating = false;
        }

        if (bIsStiffDecelerating)
        {
            ApplyStiffDeceleration(DeltaTime);
        }
    }

    FVector Velocity = OwnerCharacter ? OwnerCharacter->GetVelocity() : FVector::ZeroVector;
    if (!Velocity.IsNearlyZero())
    {
        LastVelocityRotation = Velocity.Rotation();
    }
}

void UTGFlyingComponent::EnableFlight()
{
    if (CharacterMovement)
    {
        CharacterMovement->SetMovementMode(MOVE_Flying);
        CharacterMovement->bOrientRotationToMovement = false;
        CharacterMovement->BrakingDecelerationFlying = FlyingBrakingDeceleration;
        CharacterMovement->MaxAcceleration = 6000.0f;
        CharacterMovement->MaxFlySpeed = NormalFlySpeed;
    }

    DesiredFOV = 120.0f;
    DesiredSocketOffset = FVector(150.0f, 0.0f, 90.0f);
    bIsFlying = true;
    
    APlayerController* PC = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr;
    if (PC)
    {
        PC->ClientStartCameraShake(LoadClass<UCameraShakeBase>(nullptr, TEXT("/Game/Flying/CameraShakes/CS_Flight_Loop.CS_Flight_Loop_C")), 0.1f);
        PC->ClientStartCameraShake(LoadClass<UCameraShakeBase>(nullptr, TEXT("/Game/Flying/CameraShakes/CS_Flight_Start.CS_Flight_Start_C")), 0.5f);
    }
}

void UTGFlyingComponent::DisableFlight()
{
    if (CharacterMovement)
    {
        CharacterMovement->SetMovementMode(MOVE_Falling);
        CharacterMovement->bOrientRotationToMovement = true;
        CharacterMovement->BrakingDecelerationFlying = GroundBrakingDeceleration;
        CharacterMovement->MaxAcceleration = 1500.0f;
        CharacterMovement->MaxFlySpeed = NormalFlySpeed;
    }

    DesiredFOV = 90.0f;
    DesiredSocketOffset = DefaultSocketOffset;
    bIsFlying = false;
    
    APlayerController* PC = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr;
    if (PC)
    {
        PC->ClientStopCameraShake(LoadClass<UCameraShakeBase>(nullptr, TEXT("/Game/Flying/CameraShakes/CS_Flight_Loop.CS_Flight_Loop_C")));
    }
}

void UTGFlyingComponent::Move(const FInputActionValue& Value)
{
    if (!OwnerCharacter) return;

    FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator Rotation = OwnerCharacter->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    if (bIsFlying)
    {
        const FVector ForwardDirection = Rotation.Vector();
        const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
        FVector MovementDirection = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
        MovementDirection.Normalize();
        
        OwnerCharacter->AddMovementInput(MovementDirection, 1.5f);
    }
    else
    {
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        OwnerCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
        OwnerCharacter->AddMovementInput(RightDirection, MovementVector.X);
    }
}

void UTGFlyingComponent::Look(const FInputActionValue& Value)
{
    if (!OwnerCharacter) return;

    FVector2D LookAxisVector = Value.Get<FVector2D>();
    
    OwnerCharacter->AddControllerYawInput(LookAxisVector.X);
    OwnerCharacter->AddControllerPitchInput(LookAxisVector.Y);
}

void UTGFlyingComponent::ToggleFlight()
{
    bWantsToFly = !bWantsToFly;
    
    if (bWantsToFly && !bIsFlying)
    {
        EnableFlight();
    }
    else if (!bWantsToFly && bIsFlying)
    {
        DisableFlight();
    }
}

void UTGFlyingComponent::Boost(const FInputActionValue& Value)
{
    bBoost = Value.Get<bool>();

    if (!CharacterMovement) return;

    if (bBoost)
    {
        DesiredFOV = 120.0f;
        CharacterMovement->MaxFlySpeed = BoostFlySpeed;
        CharacterMovement->BrakingDecelerationFlying = FlyingBrakingDeceleration;
        CharacterMovement->MaxAcceleration = 6000.0f;
        bIsStiffDecelerating = false;
    }
    else
    {
        DesiredFOV = 90.0f;
        CharacterMovement->MaxFlySpeed = NormalFlySpeed;
        CharacterMovement->BrakingDecelerationFlying = FlyingBrakingDeceleration;
        CharacterMovement->MaxAcceleration = 1500.0f;
        BoostDuration = 0.0f;
    }
}

void UTGFlyingComponent::UpdateCameraAndArm(float DeltaTime)
{
    if (FollowCamera && CameraBoom)
    {
        float NewFOV = FMath::FInterpTo(FollowCamera->FieldOfView, DesiredFOV, DeltaTime, 3.0f);
        FollowCamera->FieldOfView = NewFOV;

        FVector NewSocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, DesiredSocketOffset, DeltaTime, 3.0f);
        CameraBoom->SocketOffset = NewSocketOffset;
    }
}

void UTGFlyingComponent::UpdateRotation(float DeltaTime)
{
    FRotator TargetRotation = CalculateTargetRotation();
    SmoothRotation(TargetRotation, 800.0f, FlightRotationInterpSpeed, DeltaTime);
}

void UTGFlyingComponent::SmoothRotation(const FRotator& Target, float ConstantSpeed, float SmoothSpeed, float DeltaTime)
{
    if (!OwnerCharacter) return;

    FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
    FRotator InterpConstant = FMath::RInterpConstantTo(FlyingRotation, Target, DeltaTime, ConstantSpeed);
    FRotator InterpSmooth = FMath::RInterpTo(CurrentRotation, Target, DeltaTime, SmoothSpeed);
    FlyingRotation = InterpConstant;
    OwnerCharacter->SetActorRotation(InterpSmooth);
}

FRotator UTGFlyingComponent::CalculateTargetRotation() const
{
    if (ShouldUseLastVelocityRotation())
    {
        return LastVelocityRotation;
    }
    else
    {
        return OwnerCharacter ? OwnerCharacter->GetControlRotation() : FRotator::ZeroRotator;
    }
}

bool UTGFlyingComponent::ShouldUseLastVelocityRotation() const
{
    FVector Velocity = OwnerCharacter ? OwnerCharacter->GetVelocity() : FVector::ZeroVector;
    return (Velocity.Size() > 1.0f) && !bBoost;
}

bool UTGFlyingComponent::IsOnGround() const
{
    if (!CharacterMovement)
        return false;

    return CharacterMovement->IsMovingOnGround();
}

void UTGFlyingComponent::UpdateGroundDetection()
{
    if (!CharacterMovement)
        return;

    bool bWasOnGround = IsOnGround();
    bool bIsOnGround = CharacterMovement->IsMovingOnGround();

    if (bIsOnGround)
    {
        CharacterMovement->BrakingDecelerationWalking = GroundBrakingDeceleration;
        if (bIsFlying)
        {
            DisableFlight();
            bWantsToFly = false;
        }
    }
    else
    {
        CharacterMovement->BrakingDecelerationFlying = FlyingBrakingDeceleration;
        
        // If the character just left the ground and wants to fly, enable flight
        if (!bWasOnGround && bWantsToFly && !bIsFlying)
        {
            EnableFlight();
        }
    }
}

void UTGFlyingComponent::StartStiffDeceleration()
{
    bIsStiffDecelerating = true;
    if (CharacterMovement)
    {
        CharacterMovement->BrakingDecelerationFlying = FlyingBrakingDeceleration * StiffDecelerationMultiplier;
    }
}

void UTGFlyingComponent::ApplyStiffDeceleration(float DeltaTime)
{
    if (!CharacterMovement) return;

    FVector CurrentVelocity = CharacterMovement->Velocity;
    float CurrentSpeed = CurrentVelocity.Size();

    if (CurrentSpeed > NormalFlySpeed)
    {
        float DecelerationRate = CharacterMovement->BrakingDecelerationFlying * DeltaTime;
        float NewSpeed = FMath::Max(CurrentSpeed - DecelerationRate, NormalFlySpeed);
        FVector NewVelocity = CurrentVelocity.GetSafeNormal() * NewSpeed;
        CharacterMovement->Velocity = NewVelocity;
    }
    else
    {
        bIsStiffDecelerating = false;
        CharacterMovement->BrakingDecelerationFlying = FlyingBrakingDeceleration;
        BoostDuration = 0.0f;
    }
}