// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TGCustomizationHandlingManager.generated.h"

/**
 * 
 */
UCLASS()
class TOPGUN_API UTGCustomizationHandlingManager : public UObject
{
	 GENERATED_BODY()

public:
    UTGCustomizationHandlingManager();
    
    // Customizing Features
    UFUNCTION()
    static USkeletalMesh* GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void GenerateModuleButtons(UScrollBox* TargetPanel) const;

    // Spawn
    UFUNCTION(BlueprintCallable, Category = "Customizing")
    AActor* SpawnEquip(FName EquipID, APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void SpawnCurrentEquip(FName EquipID, APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void SpawnModule(FName WeaponID, APlayerController* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void AlterModuleComponent(FName WeaponID, APlayerController* Player);

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
    bool IsWithinSnapDistance(float distance, const FVector& boneLocation, FName boneName, APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void UnSnapActor();

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    bool SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName, APlayerController* Player);

    UFUNCTION(BlueprintCallable, Category = "Customizing")
    void HighlightSelectedActor(bool bEnable);

    // Rotation
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
	
};
