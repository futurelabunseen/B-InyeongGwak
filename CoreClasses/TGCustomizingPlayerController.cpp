#include "TGCustomizingPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Interface/TGCustomizingInterface.h"
#include "Utility/TGCustomizationHandlingManager.h"
#include "Utility/TGEquipmentManager.h"

ATGCustomizingPlayerController::ATGCustomizingPlayerController()
{
    bIsDragging = false;
    LastStateUpdateTime = 0.f;
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void ATGCustomizingPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!MyCustomizingComponent.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("GetMyCustomizingComponent returned null"));
    }

    EquipmentManager = Cast<UTGCGameInstance>(GetGameInstance())->GetEquipmentManager();

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    if (ACharacter* MyCharacter = GetCharacter())
    {
        MyCharacter->SetActorEnableCollision(false);
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    ACharacter* CharacterRef = GetCharacter();
    if (CharacterRef)
    {
        MainCamera = CharacterRef->FindComponentByClass<UCameraComponent>();
        if (MainCamera)
        {
            MainCamera->OrthoWidth = 500.0f;
            DefaultCameraRotation = MainCamera->GetRelativeRotation();
            DefaultCameraOffset = MainCamera->GetRelativeLocation();
            CurrentCameraOffset = DefaultCameraOffset;
        }
    }

    if (TSoftObjectPtr<UUserWidget> CurrentInventoryWidget = CreateWidget<UUserWidget>(this, InventoryWidgetTemplate))
    {
        CurrentInventoryWidget->AddToViewport();
        CurrentInventoryWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void ATGCustomizingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent);

    EnhancedInput->BindAction(LClickAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickLeftMouse);
    EnhancedInput->BindAction(LClickAction, ETriggerEvent::Started, this, &ATGCustomizingPlayerController::MouseLeftClickStarted);
    EnhancedInput->BindAction(LClickAction, ETriggerEvent::Completed, this, &ATGCustomizingPlayerController::MouseLeftClickEnded);

    EnhancedInput->BindAction(RClickAction, ETriggerEvent::Started, this, &ATGCustomizingPlayerController::OnClickRightMouse);
    EnhancedInput->BindAction(EscapeAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickEscape);

    EnhancedInput->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnRotateAction);
    EnhancedInput->BindAction(EnterAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnEnterAction);

    EnhancedInput->BindAction(EnterRotateAction, ETriggerEvent::Completed, this, &ATGCustomizingPlayerController::OnPressEnterRotateEquipment);
    EnhancedInput->BindAction(KeyBindingPressAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnPressKeyBindingEquipment);

    bBlockInput = false;
}


void ATGCustomizingPlayerController::SetCustomizingStateManager(
    ITGCustomizingInterface* CustomizingStateInterface,
    TWeakObjectPtr<UTGCustomizationHandlingManager> CustomizingComponent,
    TWeakObjectPtr<UTGCustomizingUIManager> CustomizingUIManager
)
{
    MyCustomizingStateManagerInterface = CustomizingStateInterface;
    MyCustomizingComponent = CustomizingComponent;
    MyCustimizingUIManager = CustomizingUIManager;
    UE_LOG(LogTemp, Log, TEXT("SetCustomizingStateManager called"));
}

UTGCustomizationHandlingManager* ATGCustomizingPlayerController::GetMyCustomizingComponent()
{
    return MyCustomizingComponent.Get();
}

UTGCustomizingUIManager* ATGCustomizingPlayerController::GetCustomizingUIManager()
{
    if (MyCustimizingUIManager.IsValid())
    {
        return MyCustimizingUIManager.Get();
    }
    UE_LOG(LogTemp, Warning, TEXT("GetCustomizingUIManager: MyCustimizingUIManager is null"));
    return nullptr;
}


void ATGCustomizingPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastStateUpdateTime >= StateUpdateInterval)
    {
        LastStateUpdateTime = CurrentTime;
        if (MyCustomizingStateManagerInterface)
        {
            MyCustomizingStateManagerInterface->UpdateState(this);
        }
    }
}

void ATGCustomizingPlayerController::ProcessPlayerInput(float DeltaTime, bool bGamePaused)
{
    Super::ProcessPlayerInput(DeltaTime, bGamePaused);

    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleProcessPlayerInput(this);
    }
}


void ATGCustomizingPlayerController::OnClickLeftMouse()
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleCustomizingState(ECustomizingState::OnClickActor, this);
    }
}

void ATGCustomizingPlayerController::OnClickRightMouse()
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleCustomizingState(ECustomizingState::Idle, this);
    }
    StartMouseDrag();
}

void ATGCustomizingPlayerController::OnClickEscape()
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleCustomizingState(ECustomizingState::Idle, this);
    }
}

void ATGCustomizingPlayerController::OnPressKeyBindingEquipment()
{
    UE_LOG(LogTemp, Log, TEXT("Try Entering key binding state"));
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleCustomizingState(ECustomizingState::OnBindKey, this);
    }
}

void ATGCustomizingPlayerController::OnPressEnterRotateEquipment()
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleCustomizingState(ECustomizingState::OnRotateEquip, this);
    }
}

void ATGCustomizingPlayerController::OnPressReturnToIdleState()
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleCustomizingState(ECustomizingState::Idle, this);
    }
}

void ATGCustomizingPlayerController::OnPressDeleteEquipmentAction()
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleCustomizingState(ECustomizingState::OnSelectActor, this);
    }
}

void ATGCustomizingPlayerController::OnEnterAction()
{
    if (MyCustomizingComponent.IsValid())
    {
        UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(GetWorld()->GetGameInstance());
        if (GameInstance)
        {
            GameInstance->ChangeLevel(TargetLevelName);
        }
        if (GetPawn())
        {
            GetPawn()->EndPlay(EEndPlayReason::LevelTransition);
        }
    }
}

void ATGCustomizingPlayerController::OnRotateAction(const FInputActionValue& Value)
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleRotateAction(Value);
    }
}


void ATGCustomizingPlayerController::MouseLeftClickStarted()
{
    StartMouseDrag();
}

void ATGCustomizingPlayerController::MouseLeftClickEnded()
{
    StopMouseDrag();
}

void ATGCustomizingPlayerController::StartMouseDrag()
{
    bIsDragging = true;
    GetMousePosition(MouseStartLocation.X, MouseStartLocation.Y);
}

void ATGCustomizingPlayerController::StopMouseDrag()
{
    bIsDragging = false;
}


void ATGCustomizingPlayerController::TryFindSelectActor()
{
    if (!MyCustomizingStateManagerInterface)
    {
        return;
    }

    TWeakObjectPtr<AActor> HitActor(FindTargetActorUnderMouse());
    MyCustomizingStateManagerInterface->HandleTryFindSelectActor(HitActor.Get());
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
            FVector End = Start + WorldDirection * 10000.0f;

            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(GetPawn());

            if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
            {
                return Hit.GetActor();
            }
        }
    }
    return nullptr;
}

void ATGCustomizingPlayerController::CheckSnappedCancellation()
{
    if (MyCustomizingComponent.IsValid() && MyCustomizingComponent->CurrentSpawnedActor.IsValid())
    {
        float MouseX, MouseY;
        if (GetMousePosition(MouseX, MouseY))
        {
            FVector WorldLocation, WorldDirection;
            if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
            {
                float Dist = FVector::Distance(
                    MyCustomizingComponent->CurrentSpawnedActor->GetActorLocation(),
                    WorldLocation
                );
                if (Dist > 10.0f && MyCustomizingStateManagerInterface)
                {
                    MyCustomizingComponent->UnSnapActor();
                    MyCustomizingStateManagerInterface->HandleCustomizingState(ECustomizingState::OnDragActor, this);
                }
            }
        }
    }
}


void ATGCustomizingPlayerController::OnEquipSelect(FName WeaponID)
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleEquipSelect(WeaponID, this);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OnEquipSelect: MyCustomizingStateManagerInterface is not valid"));
    }
}

void ATGCustomizingPlayerController::RemoveActorInDesiredPosition()
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleDeleteActor(this);
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

void ATGCustomizingPlayerController::ClearCurrentSpawnedEquip()
{
    if (!MyCustomizingComponent.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ClearCurrentEquipment: MyCustomizingComponent is invalid"));
        return;
    }

    if (MyCustomizingComponent->CurrentSpawnedActor.IsValid())
    {
        MyCustomizingComponent->CurrentSpawnedActor->Destroy();
        MyCustomizingComponent->CurrentSpawnedActor = nullptr;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ClearCurrentEquipment: CurrentSpawnedActor already null"));
    }
}


void ATGCustomizingPlayerController::SwitchToZoomedCamera(AActor* FocusActor)
{
    if (MainCamera && FocusActor && GetCharacter())
    {
        MainCamera->OrthoWidth = 150.0f;

        FVector CharacterLocation = GetCharacter()->GetActorLocation();
        FVector FocusLocation = FocusActor->GetActorLocation();

        FVector Offset = (FocusLocation - CharacterLocation);
        Offset.X = 25.0f; // 임의값
        CurrentCameraOffset = DefaultCameraOffset + Offset;

        MainCamera->SetRelativeLocation(CurrentCameraOffset);
        FRotator NewRotation = (FocusLocation - MainCamera->GetComponentLocation()).Rotation();
        MainCamera->SetRelativeRotation(NewRotation);
        bIsZoomedIn = true;
    }
}

void ATGCustomizingPlayerController::ReturnToDefaultCamera()
{
    UE_LOG(LogTemp, Log, TEXT("ReturnToDefaultCamera - Called"));
    if (MainCamera)
    {
        MainCamera->OrthoWidth = 500.0f;
        MainCamera->SetRelativeLocation(DefaultCameraOffset);
        MainCamera->SetRelativeRotation(DefaultCameraRotation);
        CurrentCameraOffset = DefaultCameraOffset;
        bIsZoomedIn = false;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ReturnToDefaultCamera: MainCamera is null"));
    }
}

void ATGCustomizingPlayerController::UpdateZoomedCameraPosition()
{

}


void ATGCustomizingPlayerController::BindNotationUpdatedEvent(UTextBlock* NotationTextBlock)
{
    if (NotationTextBlock)
    {
        OnNotationUpdated.AddDynamic(NotationTextBlock, &UTextBlock::SetText);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BindNotationUpdatedEvent: NotationTextBlock is null"));
    }
}

void ATGCustomizingPlayerController::UpdateNotationUI(const FString& NewText)
{
    CurrentNotation = NewText;
    UE_LOG(LogTemp, Log, TEXT("UpdateNotationUI: Setting notation to %s"), *NewText);
    OnNotationUpdated.Broadcast(FText::FromString(CurrentNotation));
}

void ATGCustomizingPlayerController::ClearNotationUI()
{
    CurrentNotation.Empty();
    OnNotationUpdated.Broadcast(FText::FromString(CurrentNotation));
}



void ATGCustomizingPlayerController::SetVisibilityCurrentWeaponToolWidget(bool bVisible)
{
    if (MyCustimizingUIManager.IsValid())
    {
        MyCustimizingUIManager->ToggleCurrentWeaponToolWidget(bVisible);
    }
}


bool ATGCustomizingPlayerController::IsValidKeyForBinding(const FKey& Key) const
{
    return ValidKeys.Contains(Key);
}

void ATGCustomizingPlayerController::FindTargetActorForKeyBind(AActor* CurrentWeapon, const FKey& Key)
{
    if (CurrentWeapon)
    {
        FEquipmentKey EquipKey = UTGEquipmentManager::GetKeyForActor(CurrentWeapon);
        EquipmentManager->BindKeyToEquipment(EquipKey, Key.GetFName());

        FString NotationText = FString::Printf(TEXT("Weapon %s bound to key %s"),
            *CurrentWeapon->GetName(),
            *Key.ToString()
        );
        UE_LOG(LogTemp, Log, TEXT("%s"), *NotationText);
        UpdateNotationUI(NotationText);
    }
}

FKey ATGCustomizingPlayerController::GetActionPressedKey(const UInputAction* Action) const
{
    if (const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
    {
        if (const UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            TArray<FKey> MappedKeys = Subsystem->QueryKeysMappedToAction(Action);
            for (const FKey& Key : MappedKeys)
            {
                if (IsInputKeyDown(Key))
                {
                    return Key;
                }
            }
        }
    }
    return FKey();
}

bool ATGCustomizingPlayerController::IsInputKeyDown(const FKey& Key) const
{
    return APlayerController::IsInputKeyDown(Key);
}

void ATGCustomizingPlayerController::OnRotateEquipment()
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
        QuatRotation *= FQuat(FRotator(0.0f, 0.0f, -PitchValue));
    }

    if (!QuatRotation.IsIdentity() && MyCustomizingComponent.IsValid())
    {
        MyCustomizingComponent->SetTargetActorRotation(QuatRotation);
    }
}


void ATGCustomizingPlayerController::UpdateEquipActorPosition()
{
    if (GetPawn() && MyCustomizingComponent.IsValid())
    {
        FVector WorldLocation, WorldDirection;
        if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        {
            MyCustomizingComponent->UpdateWeaponActorPosition(WorldLocation, WorldDirection);
        }
    }
}

void ATGCustomizingPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    OnNotationUpdated.Clear();

    if (MyCustomizingComponent.IsValid())
    {
        MyCustomizingComponent->ResetHoldingData();
    }

    Super::EndPlay(EndPlayReason);
}
