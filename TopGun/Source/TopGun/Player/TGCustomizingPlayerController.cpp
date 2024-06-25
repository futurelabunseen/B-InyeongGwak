
#include "TGCustomizingPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/ScrollBox.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"
#include "Engine/World.h"
#include "Weapon/TGBaseWeapon.h"
#include "Character/TGCustomizingCharacterBase.h"

ATGCustomizingPlayerController::ATGCustomizingPlayerController()
{
    CurrentState = ECustomizingState::Idle;
    bIsDragging = false;
}

void ATGCustomizingPlayerController::BeginPlay()
{
    Super::BeginPlay();

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    if (ACharacter* MyCharacter = GetCharacter())
    {
        MyCharacter->SetActorEnableCollision(false);
        //TEMP
        MyCustomizingComponent = MyCharacter->FindComponentByClass<UTGCustomizingComponent>();
        if (!MyCustomizingComponent)
        {
            UE_LOG(LogTemp, Error, TEXT("CustomizingComponent is not found."));
            FGenericPlatformMisc::RequestExit(false);
            return;
        }
    }
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ATGCustomizingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickLeftMouse);
    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Started, this, &ATGCustomizingPlayerController::MouseLeftClickStarted);
    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Completed, this, &ATGCustomizingPlayerController::MouseLeftClickEnded);
    EnhancedInputComponent->BindAction(RClickAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickRightMouse);
    EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickEscape);
    EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnRotateAction);
    EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnEnterAction);
}

void ATGCustomizingPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateState();
}

void ATGCustomizingPlayerController::UpdateState()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        HandleIdleState();
        break;
    case ECustomizingState::OnDragActor:
        HandleDragState();
        break;
    case ECustomizingState::OnSnappedActor:
        HandleSnappedState();
        break;
    case ECustomizingState::OnRotateActor:
        HandleRotateState();
        break;
    }
}

void ATGCustomizingPlayerController::HandleIdleState()
{
    // Logic for idle state
}

void ATGCustomizingPlayerController::HandleDragState()
{
    UpdateWeaponActorPosition();
    if (MyCustomizingComponent->IsWeaponNearBone())
    {
        EnterSnappedState();
    }
}

void ATGCustomizingPlayerController::HandleSnappedState()
{
    CheckSnappedCancellation();
}

void ATGCustomizingPlayerController::HandleRotateState()
{
    MyCustomizingComponent->DrawDebugHighlight();
}

void ATGCustomizingPlayerController::EnterIdleState()
{
    CurrentState = ECustomizingState::Idle;
}

void ATGCustomizingPlayerController::EnterDragState()
{
    CurrentState = ECustomizingState::OnDragActor;
}

void ATGCustomizingPlayerController::EnterSnappedState()
{
    CurrentState = ECustomizingState::OnSnappedActor;
}

void ATGCustomizingPlayerController::EnterRotateState()
{
    CurrentState = ECustomizingState::OnRotateActor;
}

void ATGCustomizingPlayerController::ReturnToIdleState()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        break;
    case ECustomizingState::OnDragActor:
        break;
    case ECustomizingState::OnSnappedActor:
        break;
    case ECustomizingState::OnRotateActor:
        MyCustomizingComponent->SaveRotationData();
        break;
    }
    MyCustomizingComponent->ResetHoldingData();
    EnterIdleState();
}

void ATGCustomizingPlayerController::OnRotateAction(const FInputActionValue& Value)
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnRotateActor:
        OnRotateCharacter(Value);
        break;
    }
}

void ATGCustomizingPlayerController::OnRotateCharacter(const FInputActionValue& Value)
{
    FVector2D InputVector = Value.Get<FVector2D>();
    AddYawInput(InputVector.Y);
    if (GetPawn())
    {
        ACharacter* MyCharacter = GetCharacter();
        UCameraComponent* FollowCamera = MyCharacter ? MyCharacter->FindComponentByClass<UCameraComponent>() : nullptr;
        if (FollowCamera)
        {
            float ZoomFactor = 10.0f;
            float NewOrthoWidth = FollowCamera->OrthoWidth + InputVector.X * -ZoomFactor;
            float MinOrthoWidth = 100.0f;
            float MaxOrthoWidth = 1000.0f;
            FollowCamera->OrthoWidth = FMath::Clamp(NewOrthoWidth, MinOrthoWidth, MaxOrthoWidth);
        }
    }
}

void ATGCustomizingPlayerController::OnEnterAction()
{
    MyCustomizingComponent->MyGameInstance->ChangeLevel(TargetLevelName);
}

void ATGCustomizingPlayerController::OnRotateWeaponActor()
{
    if (!bIsDragging)
    {
        return;
    }

    FVector2D MouseCurrentLocation;
    GetMousePosition(MouseCurrentLocation.X, MouseCurrentLocation.Y);

    FVector2D MouseDelta = MouseCurrentLocation - MouseStartLocation;
    MouseStartLocation = MouseCurrentLocation;

    FQuat QuatRotation = FQuat::Identity;

    if (FMath::Abs(MouseDelta.X) > KINDA_SMALL_NUMBER)
    {
        float YawValue = -MouseDelta.X * RotationSpeed * MouseSensitivity;
        QuatRotation *= FQuat(FRotator(0.0f, YawValue, 0.0f));
    }

    if (FMath::Abs(MouseDelta.Y) > KINDA_SMALL_NUMBER)
    {
        float PitchValue = MouseDelta.Y * RotationSpeed * MouseSensitivity;
        QuatRotation *= FQuat(FRotator(0.0f, 0.0f, -1.0f * PitchValue));
    }

    if (!QuatRotation.IsIdentity())
    {
        MyCustomizingComponent->SetWeaponRotation(QuatRotation);
    }
}

void ATGCustomizingPlayerController::OnClickRightMouse()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        RemoveWeaponInDesiredPosition();
        break;
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnRotateActor:
        ReturnToIdleState();
        break;
    }
}

void ATGCustomizingPlayerController::OnClickLeftMouse()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        TryFindRotatingTargetActor();
        break;
    case ECustomizingState::OnDragActor:
        break;
    case ECustomizingState::OnSnappedActor:
        if (MyCustomizingComponent->AttachWeapon())
        {
            ReturnToIdleState();
        }
        break;
    case ECustomizingState::OnRotateActor:
        OnRotateWeaponActor();
        break;
    }
}

void ATGCustomizingPlayerController::MouseLeftClickStarted()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnRotateActor:
        StartMouseDrag();
        break;
    }
}

void ATGCustomizingPlayerController::StartMouseDrag()
{
    bIsDragging = true;
    GetMousePosition(MouseStartLocation.X, MouseStartLocation.Y);
}

void ATGCustomizingPlayerController::MouseLeftClickEnded()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnRotateActor:
        StopMouseDrag();
        break;
    }
}

void ATGCustomizingPlayerController::StopMouseDrag()
{
    bIsDragging = false;
}

void ATGCustomizingPlayerController::TryFindRotatingTargetActor()
{
    AActor* TempActor = FindTargetActorUnderMouse();
    if(!TempActor)
        return;
    ATGBaseWeapon* HitWeapon = Cast<ATGBaseWeapon>(TempActor);
    if(!HitWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindingRotatingTarget Actor : %s is not Weapon"), *TempActor->GetActorNameOrLabel());
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("FindingRotatingTarget Taget : %s"), *HitWeapon->GetActorNameOrLabel());
    UE_LOG(LogTemp, Warning, TEXT("Finding Rotating Target : Found Rotation Target"));
    MyCustomizingComponent->SetCurrentRotationSelectedActor(TempActor);
    EnterRotateState();
    TempActor = nullptr;
}

AActor* ATGCustomizingPlayerController::FindTargetActorUnderMouse() const
{
    float MouseX, MouseY;
    if (GetMousePosition(MouseX, MouseY))
    {
        FVector WorldLocation, WorldDirection;
        if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        {
            FHitResult Hit;
            FVector Start = WorldLocation;
            FVector End = Start + WorldDirection * 10000;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(GetPawn());
            ECollisionChannel Channel = ECC_Visibility;
            if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Channel, QueryParams))
            {
                if (Hit.GetActor())
                {
                    return Hit.GetActor();
                }
            }
        }
    }
    return nullptr;
}

void ATGCustomizingPlayerController::RemoveWeaponInDesiredPosition()
{
    ATGBaseWeapon* HitWeapon = Cast<ATGBaseWeapon>(FindTargetActorUnderMouse());
    MyCustomizingComponent->RemoveWeaponFromCharacter(HitWeapon);
}

void ATGCustomizingPlayerController::OnClickEscape()
{
    ReturnToIdleState();
}

void ATGCustomizingPlayerController::UpdateWeaponActorPosition()
{
    if (GetPawn() && MyCustomizingComponent)
    {
        FVector WorldLocation, WorldDirection;
        if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        {
            MyCustomizingComponent->UpdateWeaponActorPosition(WorldLocation, WorldDirection);
        }
    }
}

void ATGCustomizingPlayerController::CheckSnappedCancellation()
{
    float MouseX, MouseY;
    if (GetMousePosition(MouseX, MouseY))
    {
        FVector WorldLocation, WorldDirection;
        if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        {
            if (FVector::Distance(MyCustomizingComponent->CurrentSpawnedActor->GetActorLocation(), WorldLocation) > 10)
            {
                MyCustomizingComponent->UnSnapActor();
                EnterDragState();
            }
        }
    }
}

void ATGCustomizingPlayerController::AddButtonToPanel(UScrollBox* TargetPanel, TSubclassOf<UUserWidget> TargetButtonWidget, FName ID)
{
    ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetCharacter());
    if (MyCharacter && MyCharacter->CustomizingComponent)
    {
        MyCharacter->CustomizingComponent->AddButtonToPanel(TargetPanel, TargetButtonWidget, ID);
    }
}

void ATGCustomizingPlayerController::AddWeaponButtonToPanel(UScrollBox* TargetPanel)
{
    if (MyCustomizingComponent)
    {
        MyCustomizingComponent->AddWeaponButtonToPanel(TargetPanel);
    } else
    {
        UE_LOG(LogTemp, Log, TEXT("AddWeaponButtonToPanel: CustomizingComponent null"));
    }
}

void ATGCustomizingPlayerController::AddModuleButtonToPanel(UScrollBox* TargetPanel)
{
    if (MyCustomizingComponent)
    {
        MyCustomizingComponent->AddModuleButtonToPanel(TargetPanel);
    } else
    {
        UE_LOG(LogTemp, Log, TEXT("AddModuleButtonToPanel: CustomizingComponent null"));
    }
}

void ATGCustomizingPlayerController::OnWeaponSelected(FName WeaponID)
{
    if (MyCustomizingComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("Entering OnWeaponSelected with state: %d"), static_cast<int32>(CurrentState));

        EnterIdleState();
        
        if (MyCustomizingComponent->CurrentSpawnedActor)
        {
            UE_LOG(LogTemp, Log, TEXT("Destroying CurrentSpawnedActor"));
            MyCustomizingComponent->CurrentSpawnedActor->Destroy();
            MyCustomizingComponent->CurrentSpawnedActor = nullptr;  // Reset the pointer after destruction
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CurrentSpawnedActor already null."));
        }

        MyCustomizingComponent->SpawnCurrentWeapon(WeaponID);
        EnterDragState();
        UE_LOG(LogTemp, Log, TEXT("Completed OnWeaponSelected, state: %d"), static_cast<int32>(CurrentState));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CustomizingComponent is not valid."));
    }
}

void ATGCustomizingPlayerController::OnModuleSelected(FName WeaponID)
{
    if (MyCustomizingComponent)
    {
        MyCustomizingComponent->AlterModuleComponent(WeaponID);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CustomizingComponent is not valid."));
    }
}