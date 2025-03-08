
#include "TGCustomizingPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"
#include "Engine/World.h"
#include "Interface/TGCustomizingInterface.h"
#include "Utility/TGCustomizationHandlingManager.h"
#include "Utility/TGEquipmentManager.h"

ATGCustomizingPlayerController::ATGCustomizingPlayerController()
{
    bIsDragging = false;
}

UTGCustomizationHandlingManager* ATGCustomizingPlayerController::GetMyCustomizingComponent()
{
    return MyCustomizingComponent.Get();
}

void ATGCustomizingPlayerController::SetCustomizingStateManager(ITGCustomizingInterface* CustomizingStateInterface, TWeakObjectPtr<UTGCustomizationHandlingManager> CustomizingComponent, TWeakObjectPtr<UTGCustomizingUIManager> CustomizingUIManager)
{
    MyCustomizingStateManagerInterface = CustomizingStateInterface;
    MyCustomizingComponent = CustomizingComponent;
    MyCustimizingUIManager = CustomizingUIManager;
    UE_LOG(LogTemp, Log, TEXT("SetCustomizingStateManager "));

}

void ATGCustomizingPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if(!MyCustomizingComponent.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("GetMyCustomizingComponent return was null "));
    }
    
    EquipmentManager = Cast<UTGCGameInstance>(GetGameInstance())->GetEquipmentManager();
    
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    if (ACharacter* MyCharacter = GetCharacter())
    {
        MyCharacter->SetActorEnableCollision(false);
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
    
    if (TSoftObjectPtr<UUserWidget> CurrentInventoryWidget = CreateWidget<UUserWidget>(this, InventoryWidgetTemplate))
    {
        CurrentInventoryWidget->AddToViewport();
        CurrentInventoryWidget->SetVisibility(ESlateVisibility::Visible);
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

UTGCustomizingUIManager* ATGCustomizingPlayerController::GetCustomizingUIManager()
{
    if(MyCustimizingUIManager.IsValid())
        return MyCustimizingUIManager.Get();
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GetCustomizingUIManager::MyCustimizingUIManager NULL"));
        return nullptr;
    }
}


void ATGCustomizingPlayerController::ProcessPlayerInput(float DeltaTime, bool bGamePaused)
{
    Super::ProcessPlayerInput(DeltaTime, bGamePaused);
    if(MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleProcessPlayerInput(this);
    }
}

void ATGCustomizingPlayerController::OnPressKeyBindingEquipment()
{
    UE_LOG(LogTemp, Log, TEXT("Try Entering key binding state"));

    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleOnPressKeyBindingEquipment();
    }
}

void ATGCustomizingPlayerController::OnPressEnterRotateEquipment()
{
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleOnPressEnterRotateEquipment();
    }
}


void ATGCustomizingPlayerController::OnPressReturnToIdleState()
{
    if(MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->ReturnToIdleState(this);
    }
}


bool ATGCustomizingPlayerController::IsValidKeyForBinding(const FKey& Key) const
{
    return ValidKeys.Contains(Key);
}

void ATGCustomizingPlayerController::HandleKeyBindingInput(const FKey& Key)
{
    if(MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleKeyBindingInput(Key);
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
    if (MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleOnPressDeleteEquipmentAction(this);
    }
}


void ATGCustomizingPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->UpdateState(this);
    }
}


void ATGCustomizingPlayerController::OnRotateAction(const FInputActionValue& Value)
{
    if(MyCustomizingStateManagerInterface){
        MyCustomizingStateManagerInterface->HandleRotateAction(Value);
    }
}

void ATGCustomizingPlayerController::OnRotateCameraZoom(const FInputActionValue& Value)
{
   
}

void ATGCustomizingPlayerController::FindTargetActorForKeyBind(AActor* CurrentWeapon, const FKey& Key)
{
    if (CurrentWeapon)
    {
        FEquipmentKey EquipKey = UTGEquipmentManager::GetKeyForActor(CurrentWeapon);
        EquipmentManager->BindKeyToEquipment(EquipKey, Key.GetFName());
            
        FString NotationText = FString::Printf(TEXT("Weapon %s bound to key %s"), *CurrentWeapon->GetName(), *Key.ToString());
        UE_LOG(LogTemp, Log, TEXT("%s"), *NotationText);

        UpdateNotationUI(NotationText);
    }
}

void ATGCustomizingPlayerController::UpdateNotationUI(const FString& NewText)
{
    CurrentNotation = NewText;
    UE_LOG(LogTemp, Log, TEXT("ATGCustomizingPlayerController::UpdateNotationUI -> Setting Button for EquipID: %s"), *NewText);
    OnNotationUpdated.Broadcast(FText::FromString(CurrentNotation));
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
    if (MyCustomizingComponent.IsValid())
    {
        UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(GetWorld()->GetGameInstance());
        GameInstance->ChangeLevel(TargetLevelName);
        GetPawn()->EndPlay(EEndPlayReason::LevelTransition);
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



void ATGCustomizingPlayerController::OnClickRightMouse()
{
    if(MyCustomizingStateManagerInterface)
    {
        MyCustomizingStateManagerInterface->HandleRightMouseClick(this);
        StartMouseDrag();
    }
}

void ATGCustomizingPlayerController::OnClickLeftMouse()
{
    if(MyCustomizingStateManagerInterface){
    MyCustomizingStateManagerInterface->HandleLeftMouseClick(this);
    }
}

void ATGCustomizingPlayerController::MouseLeftClickStarted()
{
    StartMouseDrag();
}

void ATGCustomizingPlayerController::StartMouseDrag()
{
    bIsDragging = true;
    GetMousePosition(MouseStartLocation.X, MouseStartLocation.Y);
}

void ATGCustomizingPlayerController::MouseLeftClickEnded()
{
    StopMouseDrag();
}

void ATGCustomizingPlayerController::StopMouseDrag()
{
    bIsDragging = false;
}

void ATGCustomizingPlayerController::TryFindSelectActor()
{
    if(MyCustomizingStateManagerInterface)
    {
        TWeakObjectPtr<AActor> HitActor(FindTargetActorUnderMouse());
        MyCustomizingStateManagerInterface->HandleTryFindSelectActor(HitActor.Get());
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
    if(MyCustomizingStateManagerInterface){
        MyCustomizingStateManagerInterface->HandleRemoveActorInDesiredPosition(this);
    }
}

void ATGCustomizingPlayerController::OnClickEscape()
{
    if(MyCustomizingStateManagerInterface){
        MyCustomizingStateManagerInterface->ReturnToIdleState(this);
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

void ATGCustomizingPlayerController::CheckSnappedCancellation()
{
    float MouseX, MouseY;
    if (GetMousePosition(MouseX, MouseY) && MyCustomizingStateManagerInterface)
    {
        FVector WorldLocation, WorldDirection;
        if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        {
            if (FVector::Distance(MyCustomizingComponent->CurrentSpawnedActor->GetActorLocation(), WorldLocation) > 10)
            {
                MyCustomizingComponent->UnSnapActor();
                MyCustomizingStateManagerInterface->EnterDragState();
            }
        }
    }
}



void ATGCustomizingPlayerController::ClearCurrentEquipment()
{
    if (!MyCustomizingComponent.Get())
    {
        UE_LOG(LogTemp, Warning, TEXT("MyCustomizingComponent is not valid or not loaded."));
        return;
    }
    
    if (MyCustomizingComponent->CurrentSpawnedActor.Get())
    {
        MyCustomizingComponent->CurrentSpawnedActor->Destroy();
        MyCustomizingComponent->CurrentSpawnedActor = nullptr;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CurrentSpawnedActor already null."));
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


void ATGCustomizingPlayerController::ClearNotationUI()
{
    CurrentNotation.Empty();
    OnNotationUpdated.Broadcast(FText::FromString(CurrentNotation));
}

void ATGCustomizingPlayerController::OnEquipSelect(FName WeaponID)
{
    if(MyCustomizingStateManagerInterface)
        MyCustomizingStateManagerInterface->HandleEquipSelect(WeaponID, this);
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TGCustomizingPlayerController::OnEquipSelect -MyCustomizingStateManagerInterface is not valid"));
    }
}

void ATGCustomizingPlayerController::SetVisibilityCurrentWeaponToolWidget(bool value)
{
    MyCustimizingUIManager->ToggleCurrentWeaponToolWidget(value);
}

