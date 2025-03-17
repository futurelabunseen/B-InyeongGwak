/**
 * ┌────────────────────────────────────────────────────────────────────────────┐
 * │ UTGCustomizationHandlingManager                                          
 * │                                                                          
 * │ Customizing 기능에서 장비를 스폰·삭제·스냅·회전시키는 핵심 로직 담당      
 * │                                                                          
 * │ 주요 역할:                                                               
 * │  ITGBaseEquipmentInterface 구현 액터(장비) 스폰 및 Bone-based 스냅 처리 
 * │  GameInstance, EquipmentManager와 연동해 장비 정보 관리                
 * │  플레이어 컨트롤러 또는 StateManager에서 호출되며 실시간 액터 관리      
 * │                                                                          
 * │ 의존성:                                                                  
 * │  UTGCGameInstance: 장비·모듈 데이터 조회                                
 * │  ITGBaseEquipmentInterface: 장비 액터와의 상호작용       
 * │       
 * │  UENUM(BlueprintType)
 * │        enum class ECustomizingState : uint8
 * │        {
 * │	    Idle,
 * │	    OnDragActor,
 * │	    OnSnappedActor,
 * │	    OnSelectActor,
 * │	    OnRotateEquip,
 * │	    OnBindKey,
 * │	    OnClickActor
 * │        };
 * │
 * └────────────────────────────────────────────────────────────────────────────┘
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Interface/TGCustomizingInterface.h"
#include "Interface/TGCustomizingPlayerInterface.h"
#include "Data/TGCustomizingTypes.h"
#include "Utility/TGCustomizationHandlingManager.h"
#include "TGCustomizingStateManager.generated.h"



UCLASS()
class TOPGUN_API UTGCustomizingStateManager
    : public UObject
    , public ITGCustomizingInterface
{
    GENERATED_BODY()

    public:
    UTGCustomizingStateManager();

    void SetPlayerController(
        ITGCustomizingPlayerInterface* InPlayerController,
        TWeakObjectPtr<UTGCustomizationHandlingManager> InCustomizingComp
    );


    virtual void HandleCustomizingState(ECustomizingState NewState, APlayerController* Player) override;
    virtual void UpdateState(APlayerController* Player) override;
    virtual void HandleTryFindSelectActor(AActor* HitActor) override;
    virtual void HandleRotateAction(const FInputActionValue& Value) override;
    virtual void HandleProcessPlayerInput(APlayerController* PC) override;
    virtual void HandleEquipSelect(FName WeaponID, APlayerController* Player) override;
    virtual void HandleDeleteActor(APlayerController* Player) override;;
private:
    void EnterIdle();
    void EnterDrag();
    void EnterSnapped();
    void EnterSelectActor();
    void EnterRotateEquip();
    void EnterBindKey();
    void ReturnToIdleState(APlayerController* Player);

    void HandleDrag(APlayerController* Player);
    void HandleSnapped();
    void HandleActorClick(APlayerController* Player);

private:
    UPROPERTY()
    TWeakObjectPtr<UTGCustomizationHandlingManager> CustomizingComponent;

    ITGCustomizingPlayerInterface* PlayerControllerInterface;
    ECustomizingState CurrentState;
};



