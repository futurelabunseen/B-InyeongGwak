#include "TGCustomizingStateManager.h"

#include "AsyncTreeDifferences.h"
#include "Interface/TGCustomizingPlayerInterface.h"
#include "Interface/TGBaseEquipmentInterface.h"
#include "InputActionValue.h"

UTGCustomizingStateManager::UTGCustomizingStateManager()
    : PlayerControllerInterface(nullptr)
    , CurrentState(ECustomizingState::Idle)
{
}

void UTGCustomizingStateManager::SetPlayerController(
    ITGCustomizingPlayerInterface* InPlayerController,
    TWeakObjectPtr<UTGCustomizationHandlingManager> InCustomizingComp
)
{
    PlayerControllerInterface = InPlayerController;
    CustomizingComponent = InCustomizingComp;
    CurrentState = ECustomizingState::Idle;
}

void UTGCustomizingStateManager::HandleCustomizingState(ECustomizingState NewState, APlayerController* Player)
{
    switch (NewState)
    {
    case ECustomizingState::Idle:         ReturnToIdleState(Player); break;
    case ECustomizingState::OnDragActor:  EnterDrag(); break;
    case ECustomizingState::OnSnappedActor: EnterSnapped(); break;
    case ECustomizingState::OnSelectActor: EnterSelectActor(); break;
    case ECustomizingState::OnRotateEquip: EnterRotateEquip(); break;
    case ECustomizingState::OnBindKey:    EnterBindKey(); break;
    case ECustomizingState::OnClickActor: HandleActorClick(Player); break;
    default: break;

    }


}

void UTGCustomizingStateManager::UpdateState(APlayerController* Player)
{
    switch (CurrentState)
    {
    case ECustomizingState::OnDragActor:  HandleDrag(Player); break;
    case ECustomizingState::OnSnappedActor: HandleSnapped(); break;
    default: break;
    }
}

void UTGCustomizingStateManager::EnterIdle()
{
    if (CustomizingComponent.IsValid())
        CustomizingComponent->HighlightSelectedActor(false);

    CurrentState = ECustomizingState::Idle;
}

void UTGCustomizingStateManager::EnterDrag()
{
    CurrentState = ECustomizingState::OnDragActor;
}

void UTGCustomizingStateManager::EnterSnapped()
{
    CurrentState = ECustomizingState::OnSnappedActor;
}

void UTGCustomizingStateManager::EnterSelectActor()
{
    CurrentState = ECustomizingState::OnSelectActor;
    if (PlayerControllerInterface)
    {
        PlayerControllerInterface->ClearCurrentSpawnedEquip();
        PlayerControllerInterface->SetVisibilityCurrentWeaponToolWidget(true);
    }
    if (CustomizingComponent.IsValid() && CustomizingComponent->GetCurrentSelectedActor())
    {
        PlayerControllerInterface->SwitchToZoomedCamera(CustomizingComponent->GetCurrentSelectedActor());
        CustomizingComponent->HighlightSelectedActor(true);
    }
}

void UTGCustomizingStateManager::EnterRotateEquip()
{
    CurrentState = ECustomizingState::OnRotateEquip;
}

void UTGCustomizingStateManager::EnterBindKey()
{
    CurrentState = ECustomizingState::OnBindKey;
}

void UTGCustomizingStateManager::ReturnToIdleState(APlayerController* Player)
{
    switch (CurrentState)
    {
    case ECustomizingState::OnSelectActor:
    case ECustomizingState::OnRotateEquip:
    case ECustomizingState::OnBindKey:
        if (PlayerControllerInterface)
        {
            PlayerControllerInterface->ReturnToDefaultCamera();
            PlayerControllerInterface->SetVisibilityCurrentWeaponToolWidget(false);
        }
        if (CustomizingComponent.IsValid())
        {
            CustomizingComponent->SaveRotationData(Player);
        }
        if (PlayerControllerInterface)
        {
            PlayerControllerInterface->ClearNotationUI();
        }
        break;
    default:
        break;
    }

    if (CustomizingComponent.IsValid())
    {
        CustomizingComponent->ResetHoldingData();
    }
    EnterIdle();
}



void UTGCustomizingStateManager::HandleDrag(APlayerController* Player)
{
    if (!PlayerControllerInterface) return;
    PlayerControllerInterface->UpdateEquipActorPosition();

    if (CustomizingComponent.IsValid() && CustomizingComponent->IsEquipNearBone(Player))
        EnterSnapped();
}

void UTGCustomizingStateManager::HandleSnapped()
{
    if (PlayerControllerInterface)
        PlayerControllerInterface->CheckSnappedCancellation();
}

void UTGCustomizingStateManager::HandleActorClick(APlayerController* Player)
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        PlayerControllerInterface->TryFindSelectActor();
        break;
    case ECustomizingState::OnSnappedActor:
        if (CustomizingComponent.IsValid() && CustomizingComponent->AttachActor(Player))
        {
            EnterIdle();
        }
        break;
    case ECustomizingState::OnRotateEquip:
        PlayerControllerInterface->OnRotateEquipment();
        break;
    default:
        break;
    }
}



void UTGCustomizingStateManager::HandleTryFindSelectActor(AActor* HitActor)
{
    if (HitActor && HitActor->Implements<UTGBaseEquipmentInterface>())
    {
        if (CustomizingComponent.IsValid() && CustomizingComponent->SetCurrentSelectedActor(HitActor))
        {
            EnterSelectActor();
        }
    }
}

void UTGCustomizingStateManager::HandleRotateAction(const FInputActionValue& Value)
{
    if (!PlayerControllerInterface) return;

    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
        PlayerControllerInterface->OnRotateCharacter(Value);
        break;
    case ECustomizingState::OnSelectActor:
    case ECustomizingState::OnRotateEquip:
        PlayerControllerInterface->OnRotateCameraZoom(Value);
        break;
    default:
        break;
    }
}



void UTGCustomizingStateManager::HandleProcessPlayerInput(APlayerController* PC)
{
    if (CurrentState == ECustomizingState::OnBindKey && PC)
    {
        TArray<FKey> AllKeys;
        EKeys::GetAllKeys(AllKeys);
        for (const FKey& Key : AllKeys)
        {
            if (Key != EKeys::AnyKey && PC->WasInputKeyJustPressed(Key))
            {
                if (PlayerControllerInterface && PlayerControllerInterface->IsValidKeyForBinding(Key))
                {
                    if (CustomizingComponent.IsValid())
                    {
                        AActor* CurEquip = CustomizingComponent->GetCurrentSelectedActor();
                        PlayerControllerInterface->FindTargetActorForKeyBind(CurEquip, Key);
                    }
                    EnterSelectActor();
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Invalid key for weapon binding: %s"), *Key.ToString());
                }
                break;
            }
        }
    }
}

void UTGCustomizingStateManager::HandleEquipSelect(FName WeaponID, APlayerController* Player)
{
    EnterIdle();
    PlayerControllerInterface->ClearCurrentSpawnedEquip();

    CustomizingComponent->SpawnCurrentEquip(WeaponID, Player);
    EnterDrag();
}

void UTGCustomizingStateManager::HandleDeleteActor(APlayerController* Player)
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