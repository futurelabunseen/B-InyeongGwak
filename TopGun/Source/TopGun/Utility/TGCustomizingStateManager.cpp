#include "TGCustomizingStateManager.h"
#include "InputActionValue.h"
#include "TGCustomizationHandlingManager.h"
#include "Interface/TGBaseEquipmentInterface.h"
#include "Interface/TGCustomizingPlayerInterface.h"

UTGCustomizingStateManager::UTGCustomizingStateManager()
    : CustomizingComponent(nullptr)
    , CurrentState(ECustomizingState::Idle)
{
}

void UTGCustomizingStateManager::SetPlayerController(ITGCustomizingPlayerInterface* InPlayerController,  TWeakObjectPtr<UTGCustomizationHandlingManager>  InCustomizingComponent)
{
    PlayerControllerInterface = InPlayerController;
    CurrentState = ECustomizingState::Idle;
    CustomizingComponent = InCustomizingComponent;
}

void UTGCustomizingStateManager::UpdateState(APlayerController* Player)
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        HandleIdleState();
        break;
    case ECustomizingState::OnDragActor:
        HandleDragState(Player);
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

void UTGCustomizingStateManager::EnterIdleState()
{
    if (CustomizingComponent.IsValid())
    {
        CustomizingComponent->HighlightSelectedActor(false);
    }
    CurrentState = ECustomizingState::Idle;
}

void UTGCustomizingStateManager::EnterDragState()
{
    CurrentState = ECustomizingState::OnDragActor;
}

void UTGCustomizingStateManager::EnterSnappedState()
{
    CurrentState = ECustomizingState::OnSnappedActor;
}

void UTGCustomizingStateManager::EnterRotateState()
{
    CurrentState = ECustomizingState::OnRotateEquip;
}

void UTGCustomizingStateManager::EnterSelectActorState()
{
    CurrentState = ECustomizingState::OnSelectActor;
    PlayerControllerInterface->ClearCurrentEquipment();
    PlayerControllerInterface->SetVisibilityCurrentWeaponToolWidget(true);

    if(CustomizingComponent->GetCurrentSelectedActor())
        PlayerControllerInterface->SwitchToZoomedCamera(CustomizingComponent->GetCurrentSelectedActor());
    
    CustomizingComponent->HighlightSelectedActor(true);
}

void UTGCustomizingStateManager::EnterBindKeyState()
{
    CurrentState = ECustomizingState::OnBindKey;
}

void UTGCustomizingStateManager::ReturnToSelectActorState()
{
    CurrentState = ECustomizingState::OnSelectActor;
}

void UTGCustomizingStateManager::ReturnToIdleState(APlayerController* Player)
{
    switch (CurrentState)
    {
    case ECustomizingState::OnSelectActor:
    case ECustomizingState::OnRotateEquip:
    case ECustomizingState::OnBindKey:
        PlayerControllerInterface->ReturnToDefaultCamera();
        PlayerControllerInterface->SetVisibilityCurrentWeaponToolWidget(false);
        CustomizingComponent->SaveRotationData(Player);
        PlayerControllerInterface->ClearNotationUI();
        break;
    }
    
    if (CustomizingComponent.IsValid())
    {
        CustomizingComponent->ResetHoldingData();
    }
    EnterIdleState();
}

void UTGCustomizingStateManager::HandleIdleState()
{
}

void UTGCustomizingStateManager::HandleDragState(APlayerController* Player)
{
    PlayerControllerInterface->UpdateEquipActorPosition();
    if (CustomizingComponent->IsEquipNearBone(Player))
    {
        EnterSnappedState();
    }
}

void UTGCustomizingStateManager::HandleSnappedState()
{
    PlayerControllerInterface->CheckSnappedCancellation();
}

void UTGCustomizingStateManager::HandleRotateState()
{
}

void UTGCustomizingStateManager::HandleSelectActorState()
{
}

void UTGCustomizingStateManager::HandleBindKeyState()
{
}

void UTGCustomizingStateManager::HandleRightMouseClick(APlayerController* Player)
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        break;
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnSelectActor:
    case ECustomizingState::OnRotateEquip:
        ReturnToIdleState(Player);
        break;
    default:
        break;
    }
}

void UTGCustomizingStateManager::HandleLeftMouseClick(APlayerController* Player)
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        PlayerControllerInterface->TryFindSelectActor();
        break;
    case ECustomizingState::OnDragActor:
        break;
    case ECustomizingState::OnSnappedActor:
        if (CustomizingComponent->AttachActor(Player))
        {
            ReturnToIdleState(Player);
        }
        break;
    case ECustomizingState::OnRotateEquip:
        PlayerControllerInterface->OnRotateEquipment();
        break;
    }
}
void UTGCustomizingStateManager::HandleEnterRotateEquipment()
{
    if (CurrentState == ECustomizingState::OnSelectActor)
    {
        CustomizingComponent->HighlightSelectedActor(true);
        EnterRotateState();
    }
    else if (CurrentState == ECustomizingState::OnRotateEquip)
    {
        EnterSelectActorState();
    }
}

void UTGCustomizingStateManager::HandleKeyBindingEquipment()
{
    if (CurrentState == ECustomizingState::OnSelectActor)
    {
        EnterBindKeyState();
    }
}

void UTGCustomizingStateManager::HandleDeleteEquipmentAction(APlayerController* Player)
{
    if (CurrentState == ECustomizingState::OnSelectActor)
    {
        if (CustomizingComponent.IsValid())
        {
            AActor* CurrentSelectedActor = CustomizingComponent->GetCurrentSelectedActor();
            if (CurrentSelectedActor && CurrentSelectedActor->Implements<UTGBaseEquipmentInterface>())
            {
                CustomizingComponent->RemoveEquipFromCharacter(CurrentSelectedActor, Player);
                ReturnToIdleState(Player);
            }
        }
    }
}


void UTGCustomizingStateManager::HandleRotateAction(const FInputActionValue& Value)
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
        PlayerControllerInterface->OnRotateCharacter(Value);
        break;
    case ECustomizingState::OnRotateEquip:
    case ECustomizingState::OnSelectActor:
        PlayerControllerInterface->OnRotateCameraZoom(Value);
        break;
    default:
        break;
    }
}

void UTGCustomizingStateManager::HandleEquipSelect(FName WeaponID, APlayerController* Player)
{
    EnterIdleState();
    PlayerControllerInterface->ClearCurrentEquipment();
   
    CustomizingComponent->SpawnCurrentEquip(WeaponID, Player);
    EnterDragState();
}

void UTGCustomizingStateManager::HandleTryFindSelectActor(AActor* HitActor)
{
    if (HitActor && HitActor->Implements<UTGBaseEquipmentInterface>())
    {
        if (CustomizingComponent.IsValid() && CustomizingComponent->SetCurrentSelectedActor(HitActor))
        {
            EnterSelectActorState();
        }
    }
}

void UTGCustomizingStateManager::HandleRemoveActorInDesiredPosition(APlayerController* Player)
{
    if (CustomizingComponent.IsValid())
    {
        AActor* CurrentSelectedActor = CustomizingComponent->GetCurrentSelectedActor();
        if (CurrentSelectedActor && CurrentSelectedActor->Implements<UTGBaseEquipmentInterface>())
        {
            CustomizingComponent->RemoveEquipFromCharacter(CurrentSelectedActor, Player);
            ReturnToIdleState(Player);
        }
    }
}

void UTGCustomizingStateManager::HandleProcessPlayerInput(APlayerController* PC)
{
    if (CurrentState == ECustomizingState::OnBindKey)
    {
        TArray<FKey> AllKeys;
        EKeys::GetAllKeys(AllKeys);

        for (const FKey& Key : AllKeys)
        {
            if (Key != EKeys::AnyKey && PC->WasInputKeyJustPressed(Key))
            {
                HandleKeyBindingInput(Key);
                break;
            }
        }
    }
}

void UTGCustomizingStateManager::HandleKeyBindingInput(const FKey& Key)
{
    if (PlayerControllerInterface->IsValidKeyForBinding(Key))
    {
        UE_LOG(LogTemp, Log, TEXT("Key pressed for binding: %s"), *Key.ToString());
        
        AActor* CurrentWeapon = CustomizingComponent->GetCurrentSelectedActor();
        PlayerControllerInterface->FindTargetActor(CurrentWeapon, Key);
        ReturnToSelectActorState();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid key for weapon binding: %s"), *Key.ToString());
    }
}

void UTGCustomizingStateManager::HandleOnPressKeyBindingEquipment()
{
    if (CurrentState == ECustomizingState::OnSelectActor)
    {
        EnterBindKeyState();
    }
}

void UTGCustomizingStateManager::HandleOnPressEnterRotateEquipment()
{
    if (CurrentState == ECustomizingState::OnSelectActor)
    {
        if (CustomizingComponent.IsValid())
        {
            CustomizingComponent->HighlightSelectedActor(true);
        }
        EnterRotateState();
    }
    else if (CurrentState == ECustomizingState::OnRotateEquip)
    {
        EnterSelectActorState();
    }
}

void UTGCustomizingStateManager::HandleOnPressDeleteEquipmentAction(APlayerController* Player)
{
    if (CurrentState == ECustomizingState::OnSelectActor)
    {
        HandleDeleteEquipmentAction(Player);
    }
}