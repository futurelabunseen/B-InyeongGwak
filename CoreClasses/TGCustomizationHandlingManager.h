/**
 * ┌────────────────────────────────────────────────────────────────────────────┐
 * │ UTGCustomizingStateManager                                               
 * │                                                                          
 * │ Customizing 상태 전환과 흐름 제어를 담당합니다.                           
 * │                                                                          
 * │ 주요 역할:                                                               
 * │  ECustomizingState에 따라 액터 이동, 스냅, 로테이션 등의 로직 분기      
 * │  PlayerControllerInterface와 CustomizingComponent 간 상호작용 조율      
 * │  Select, Drag, Snapped 등 여러 상태  전환·관리                 
 * │                                                                          
 * │ 의존성:                                                                  
 * │  PlayerControllerInterface: 유저 인풋 및 UI 연동                        
 * │  CustomizingComponent: 장비 스폰·부착·삭제 기능                         
 * └────────────────────────────────────────────────────────────────────────────┘
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameInstance/TGGameInstance.h"
#include "TGCustomizationHandlingManager.generated.h"

UCLASS()
class TOPGUN_API UTGCustomizationHandlingManager : public UObject
{
    GENERATED_BODY()

    public:
    UTGCustomizationHandlingManager();

    // Customizing Features
    UFUNCTION()
    static USkeletalMesh* GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset);
    // Spawn Functions
    UFUNCTION(BlueprintCallable, Category = "Customizing")
    AActor* SpawnEquip(FName EquipID, APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void SpawnCurrentEquip(FName EquipID, APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void SpawnModule(FName WeaponID, APlayerController* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void AlterModuleComponent(FName WeaponID, APlayerController* Player);

    // Actor Attachment & Registration
    UFUNCTION(BlueprintCallable, Category = "Customizing")
    bool AttachActor(APlayerController* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    bool EquipRegister(AActor* ClonedActor, APlayerController* Player) const;

    // Handling Spawned Actor
    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void RemoveEquipFromCharacter(AActor* EquipToRemove, APlayerController* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void UpdateWeaponActorPosition(const FVector& WorldLocation, const FVector& WorldDirection) const;

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    bool IsEquipNearBone(APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    bool IsWithinSnapDistance(float Distance, const FVector& BoneLocation, FName BoneName, APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void UnSnapActor();

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    bool SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName, APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void HighlightSelectedActor(bool bEnable);

    // Rotation & Selection
    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void SaveRotationData(APlayerController* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void ResetHoldingData();

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void SetTargetActorRotation(FQuat Rotation) const;

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    bool SetCurrentSelectedActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    AActor* GetCurrentSelectedActor() const;

    // Debug
    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void DrawDebugHighlight() const;

public:
    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentSpawnedActor;

private:
    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentSelectedActor;

    FName CurrentTargetBone;
    bool bIsHighlighted;
    FGuid InstanceID;

    UPROPERTY(EditAnywhere)
    float SnapCheckDistance = 50.0f;

    FORCEINLINE UTGCGameInstance* GetGameInstance(const APlayerController* const Player) const
    {
        return Cast<UTGCGameInstance>(Player->GetWorld()->GetGameInstance());
    }

    FORCEINLINE USkeletalMeshComponent* GetCharacterMesh(const APlayerController* const Player) const
    {
        return Player->GetCharacter()->GetMesh();
    }

    FORCEINLINE ACharacter* GetCharacterFromPlayer(const APlayerController* const Player) const
    {
        return Player ? Player->GetCharacter() : nullptr;
    }
};
