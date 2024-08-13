
#include "TGCustomizingPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Equip/TGBaseArmour.h"
#include "Camera/CameraComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"
#include "Engine/World.h"
#include "Equip/TGBaseWeapon.h"
#include "Utility/TGEquipmentManager.h"

ATGCustomizingPlayerController::ATGCustomizingPlayerController()
{
    CurrentState = ECustomizingState::Idle;
    bIsDragging = false;
}

void ATGCustomizingPlayerController::BeginPlay()
{
    Super::BeginPlay();
    EquipmentManager = Cast<UTGCGameInstance>(GetGameInstance())->GetEquipmentManager();
    
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    if (ACharacter* MyCharacter = GetCharacter())
    {
        MyCharacter->SetActorEnableCollision(false);
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

    ACharacter* MyCharacter = GetCharacter();
    if (MyCharacter)
    {
        MainCamera = MyCharacter->FindComponentByClass<UCameraComponent>();
        MainCamera->OrthoWidth = 500.0f;
        DefaultCameraRotation = MainCamera->GetRelativeRotation();
        DefaultCameraOffset = MainCamera->GetRelativeLocation();
        CurrentCameraOffset = DefaultCameraOffset;
    }

    CurrentInventoryWidget = CreateWidget<UUserWidget>(this, InventoryWidgetTemplate);
    if (CurrentInventoryWidget)
    {
        CurrentInventoryWidget->AddToViewport();
        CurrentInventoryWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void ATGCustomizingPlayerController::ProcessPlayerInput(float DeltaTime, bool bGamePaused)
{
    Super::ProcessPlayerInput(DeltaTime, bGamePaused);

    if (CurrentState == ECustomizingState::OnBindKey)
    {
        TArray<FKey> AllKeys;
        EKeys::GetAllKeys(AllKeys);

        for (const FKey& Key : AllKeys)
        {
            if (Key != EKeys::AnyKey && WasInputKeyJustPressed(Key))
            {
                HandleKeyBindingInput(Key);
                break;
            }
        }
    }
}

void ATGCustomizingPlayerController::OnPressKeyBindingEquipment()
{
    UE_LOG(LogTemp, Log, TEXT("Try Entering key binding state"));

    if (CurrentState == ECustomizingState::OnSelectActor)
    {
        EnterBindKeyState();
        UE_LOG(LogTemp, Log, TEXT("Entered key binding state"));
    }
}

void ATGCustomizingPlayerController::EnterBindKeyState()
{
    CurrentState = ECustomizingState::OnBindKey;
    UE_LOG(LogTemp, Log, TEXT("Entered key binding state"));
}

bool ATGCustomizingPlayerController::IsValidKeyForBinding(const FKey& Key) const
{
    return ValidKeys.Contains(Key);
}

void ATGCustomizingPlayerController::HandleKeyBindingInput(const FKey& Key)
{
    if (IsValidKeyForBinding(Key))
    {
        UE_LOG(LogTemp, Log, TEXT("Key pressed for binding: %s"), *Key.ToString());
        
        AActor* CurrentWeapon = MyCustomizingComponent->GetCurrentSelectedActor();
        if (CurrentWeapon)
        {
            FEquipmentKey EquipKey = UTGEquipmentManager::GetKeyForActor(CurrentWeapon);
            EquipmentManager->BindKeyToEquipment(EquipKey, Key.GetFName());
            
            FString NotationText = FString::Printf(TEXT("Weapon %s bound to key %s"), *CurrentWeapon->GetName(), *Key.ToString());
            UE_LOG(LogTemp, Log, TEXT("%s"), *NotationText);
            UpdateNotationUI(NotationText);
        }
        ReturnToSelectActorState();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid key for weapon binding: %s"), *Key.ToString());
    }
}

void ATGCustomizingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickLeftMouse);
    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Started, this, &ATGCustomizingPlayerController::MouseLeftClickStarted);
    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Completed, this, &ATGCustomizingPlayerController::MouseLeftClickEnded);
    EnhancedInputComponent->BindAction(RClickAction, ETriggerEvent::Started, this, &ATGCustomizingPlayerController::OnClickRightMouse);
    EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickEscape);
    EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnRotateAction);
    EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnEnterAction);
    EnhancedInputComponent->BindAction(EnterRotateAction, ETriggerEvent::Completed, this, &ATGCustomizingPlayerController::OnPressEnterRotateEquipment);
    EnhancedInputComponent->BindAction(KeyBindingPressAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnPressKeyBindingEquipment);
    EnhancedInputComponent->BindAction(DeleteEquipmentAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnPressDeleteEquipmentAction);

    bBlockInput = false;
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





void ATGCustomizingPlayerController::OnPressDeleteEquipmentAction()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        break;
    case ECustomizingState::OnDragActor:
        break;
    case ECustomizingState::OnSnappedActor:
        break;
    case ECustomizingState::OnSelectActor:
        RemoveActorInDesiredPosition();
        break;
    case ECustomizingState::OnRotateEquip:
        break;
    }
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
    case ECustomizingState::OnSelectActor:
        HandleSelectActorState();
        break;
    case ECustomizingState::OnRotateEquip:
        HandleRotateState();
        break;
    case ECustomizingState::OnBindKey:
        HandleBindKeyState();
        break;
    }
}

void ATGCustomizingPlayerController::HandleIdleState()
{
    // Logic for idle state
}

void ATGCustomizingPlayerController::HandleDragState()
{
    UpdateEquipActorPosition();
    if (MyCustomizingComponent->IsEquipNearBone())
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
    //MyCustomizingComponent->DrawDebugHighlight();
}

void ATGCustomizingPlayerController::HandleBindKeyState()
{
}

void ATGCustomizingPlayerController::EnterIdleState()
{
    MyCustomizingComponent->HighlightSelectedActor(false);
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
    CurrentState = ECustomizingState::OnRotateEquip;
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
    case ECustomizingState::OnSelectActor:
        ReturnToDefaultCamera();
        if(CurrentWeaponToolWidget)
            CurrentWeaponToolWidget->SetVisibility(ESlateVisibility::Hidden);
        MyCustomizingComponent->SaveRotationData();
        ClearNotationUI();
        break;
    case ECustomizingState::OnRotateEquip:
        ReturnToDefaultCamera();
        if(CurrentWeaponToolWidget)
            CurrentWeaponToolWidget->SetVisibility(ESlateVisibility::Hidden);
        MyCustomizingComponent->SaveRotationData();
        ClearNotationUI();
        break;
    case ECustomizingState::OnBindKey:
        ReturnToDefaultCamera();
        if(CurrentWeaponToolWidget)
            CurrentWeaponToolWidget->SetVisibility(ESlateVisibility::Hidden);
        ClearNotationUI();
        break;
    }
    MyCustomizingComponent->ResetHoldingData();
    EnterIdleState();
}

void ATGCustomizingPlayerController::EnterSelectActorState()
{
    CurrentState = ECustomizingState::OnSelectActor;
    ClearCurrentEquipment();
    if(CurrentWeaponToolWidget) 
        CurrentWeaponToolWidget->SetVisibility(ESlateVisibility::Visible);

    if(MyCustomizingComponent->GetCurrentSelectedActor())
        SwitchToZoomedCamera(MyCustomizingComponent->GetCurrentSelectedActor());
    
    MyCustomizingComponent->HighlightSelectedActor(true);
}

void ATGCustomizingPlayerController::ReturnToSelectActorState()
{
    CurrentState = ECustomizingState::OnSelectActor;
}

void ATGCustomizingPlayerController::HandleSelectActorState()
{
    //To be added
}


void ATGCustomizingPlayerController::OnRotateAction(const FInputActionValue& Value)
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
        OnRotateCharacter(Value);
    case ECustomizingState::OnRotateEquip:
    case ECustomizingState::OnSelectActor:
        OnRotateCameraZoom(Value);
        
        break;
    }
    
}

void ATGCustomizingPlayerController::OnRotateCameraZoom(const FInputActionValue& Value)
{
   
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



void ATGCustomizingPlayerController::OnPressEnterRotateEquipment()
{
    if (CurrentState == ECustomizingState::OnSelectActor)
    {
        MyCustomizingComponent->HighlightSelectedActor(true);
        EnterRotateState();
    } else if (CurrentState == ECustomizingState::OnRotateEquip)
    {
        EnterSelectActorState();
    }
}


void ATGCustomizingPlayerController::OnPressReturnToIdleState()
{
    ReturnToIdleState();
}



void ATGCustomizingPlayerController::OnEnterAction()
{
    if (MyCustomizingComponent && MyCustomizingComponent->MyGameInstance.IsValid())
    {
        MyCustomizingComponent->MyGameInstance->ChangeLevel(TargetLevelName);
        MyCustomizingComponent->EndPlay(EEndPlayReason::LevelTransition);
    }
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
        QuatRotation *= FQuat(FRotator(0.0f, 0.0f, -1.0f * PitchValue));
    }

    if (!QuatRotation.IsIdentity())
    {
        MyCustomizingComponent->SetTargetActorRotation(QuatRotation);
    }
}

void ATGCustomizingPlayerController::BindNotationUpdatedEvent(UTextBlock* NotationTextBlock)
{
    if (NotationTextBlock)
    {
        OnNotationUpdated.AddDynamic(NotationTextBlock, &UTextBlock::SetText);
    } else
    {
        UE_LOG(LogTemp, Warning, TEXT("BindNotationUpdatedEvent(UTextBlock* NotationTextBlock)::NotationTextBlock NULL"));

    }
}

void ATGCustomizingPlayerController::OnClickRightMouse()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        //RemoveActorInDesiredPosition();
        break;
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnSelectActor:
        ReturnToIdleState();
    case ECustomizingState::OnRotateEquip:
        ReturnToIdleState();
        break;
    }
}

void ATGCustomizingPlayerController::OnClickLeftMouse()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        TryFindSelectActor();
        break;
    case ECustomizingState::OnDragActor:
        break;
    case ECustomizingState::OnSnappedActor:
        if (MyCustomizingComponent->AttachActor())
        {
            ReturnToIdleState();
        }
        break;
    case ECustomizingState::OnRotateEquip:
        OnRotateEquipment();
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
    case ECustomizingState::OnRotateEquip:
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
    case ECustomizingState::OnRotateEquip:
        StopMouseDrag();
        break;
    }
}

void ATGCustomizingPlayerController::StopMouseDrag()
{
    bIsDragging = false;
}

void ATGCustomizingPlayerController::TryFindSelectActor()
{
    TWeakObjectPtr<AActor> HitActor (FindTargetActorUnderMouse());
    if(!HitActor->Implements<UTGBaseEquipmentInterface>()){

        UE_LOG(LogTemp, Warning, TEXT("No actor found under mouse."));
        return;
    }
    if (MyCustomizingComponent->SetCurrentSelectedActor(HitActor.Get()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Entered Selected State with actor: %s"), *HitActor.Get()->GetName());
        EnterSelectActorState();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to set Selected actor."));
        return;
    }
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

void ATGCustomizingPlayerController::RemoveActorInDesiredPosition()
{
    if (!MyCustomizingComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("RemoveActorInDesiredPosition: MyCustomizingComponent is null"));
        return;
    }

    AActor* CurrentSelectedActor = MyCustomizingComponent->GetCurrentSelectedActor();
    if (!CurrentSelectedActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveActorInDesiredPosition: No actor currently selected"));
        return;
    }

    if (!CurrentSelectedActor->Implements<UTGBaseEquipmentInterface>())
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveActorInDesiredPosition: Selected actor does not implement TGBaseEquipmentInterface"));
        return;
    }

    MyCustomizingComponent->RemoveEquipFromCharacter(CurrentSelectedActor);
    ReturnToIdleState();
}

void ATGCustomizingPlayerController::OnClickEscape()
{
    ReturnToIdleState();
}

void ATGCustomizingPlayerController::UpdateEquipActorPosition()
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


void ATGCustomizingPlayerController::AddWeaponButtonToPanel(UScrollBox* TargetPanel)
{
    if (MyCustomizingComponent)
    {
        MyCustomizingComponent->GenerateEquipButtons(TargetPanel, ETGEquipmentCategory::Weapon);
    } else
    {
        UE_LOG(LogTemp, Log, TEXT("AddWeaponButtonToPanel: CustomizingComponent null"));
    }
}

void ATGCustomizingPlayerController::AddModuleButtonToPanel(UScrollBox* TargetPanel)
{
    if (MyCustomizingComponent)
    {
        MyCustomizingComponent->GenerateModuleButtons(TargetPanel);
    } else
    {
        UE_LOG(LogTemp, Log, TEXT("AddModuleButtonToPanel: CustomizingComponent null"));
    }
}

void ATGCustomizingPlayerController::AddArmourButtonToPanel(UScrollBox* TargetPanel)
{
    if (MyCustomizingComponent)
    {
        MyCustomizingComponent->GenerateEquipButtons(TargetPanel, ETGEquipmentCategory::Armour);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AddArmourButtonToPanel: CustomizingComponent null"));
    }
}

void ATGCustomizingPlayerController::OnEquipSelect(FName WeaponID)
{
    if (MyCustomizingComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("Entering OnEquipSelected with state: %d"), static_cast<int32>(CurrentState));

        EnterIdleState();
        
        ClearCurrentEquipment();

        MyCustomizingComponent->SpawnCurrentEquip(WeaponID);
        UE_LOG(LogTemp, Log, TEXT("Setting Button for EquipID: %s"), *WeaponID.ToString());
        EnterDragState();
        UE_LOG(LogTemp, Log, TEXT("Completed OnWeaponSelected, state: %d"), static_cast<int32>(CurrentState));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CustomizingComponent is not valid."));
    }
}

void ATGCustomizingPlayerController::ClearCurrentEquipment() const
{
    if (MyCustomizingComponent->CurrentSpawnedActor)
    {
        MyCustomizingComponent->CurrentSpawnedActor->Destroy();
        MyCustomizingComponent->CurrentSpawnedActor = nullptr;  // Reset the pointer after destruction
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CurrentSpawnedActor already null."));
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

void ATGCustomizingPlayerController::SwitchToZoomedCamera(AActor* FocusActor)
{
    if (MainCamera && FocusActor && GetCharacter())
    {
        MainCamera->OrthoWidth = 150.0f;
        FVector CharacterLocation = GetCharacter()->GetActorLocation();
        FVector FocusLocation = FocusActor->GetActorLocation();
        FVector Offset = FocusLocation - CharacterLocation;
        Offset.X = 25.0f;
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
        MainCamera->SetRelativeRotation(DefaultCameraRotation);  // 수정된 부분
        CurrentCameraOffset = DefaultCameraOffset;
        bIsZoomedIn = false;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ReturnToDefaultCamera - MainCamera is null"));
    }
}

void ATGCustomizingPlayerController::UpdateZoomedCameraPosition()
{
   
}

void ATGCustomizingPlayerController::RegisterWeaponSelectButton(UUserWidget* TargetWidget)
{
    CurrentWeaponToolWidget = TargetWidget;
    if(CurrentWeaponToolWidget)
        CurrentWeaponToolWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ATGCustomizingPlayerController::UpdateNotationUI(const FString& NewText)
{
    CurrentNotation = NewText;
    UE_LOG(LogTemp, Log, TEXT("ATGCustomizingPlayerController::UpdateNotationUI -> Setting Button for EquipID: %s"), *NewText);
    OnNotationUpdated.Broadcast(FText::FromString(CurrentNotation));
}

void ATGCustomizingPlayerController::ClearNotationUI()
{
    CurrentNotation.Empty();
    OnNotationUpdated.Broadcast(FText::FromString(CurrentNotation));
}