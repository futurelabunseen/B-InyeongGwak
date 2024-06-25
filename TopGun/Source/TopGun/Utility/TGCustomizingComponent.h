#pragma once

#include "CoreMinimal.h"
#include "TGWeaponDataAsset.h"
#include "Components/ActorComponent.h"
#include "Utility/TGModuleSystem.h"
#include "TGCustomizingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOPGUN_API UTGCustomizingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTGCustomizingComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWidget")
    TSubclassOf<UUserWidget> ModuleButtonWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWidget")
    TSubclassOf<UUserWidget> WeaponButtonWidgetClass;

    void AddButtonToPanel(class UScrollBox* TargetPanel, TSubclassOf<class UUserWidget> TargetButtonWidget, FName ID) const;
    void AddWeaponButtonToPanel(UScrollBox* TargetPanel) const;
    void AddModuleButtonToPanel(UScrollBox* TargetPanel) const;

    // Customizing Features
    static USkeletalMesh* GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset);
    static UBlueprintGeneratedClass* GetWeaponClassById(FName WeaponID, UTGWeaponDataAsset* WeaponDataAsset);

    // Spawn
    AActor* SpawnWeapon(FName WeaponID);
    void SpawnCurrentWeapon(FName WeaponID);
    void SpawnModule(FName WeaponID) const; // MeshMerge Method1
    void AlterModuleComponent(FName WeaponID); // LeaderPoseComponent Method2
    bool AttachWeapon() const;

    // Handling Spawned Actor
    void RemoveWeaponFromCharacter(AActor* WeaponToRemove) const;
    void UpdateWeaponActorPosition(const FVector& WorldLocation, const FVector& WorldDirection) const;

    bool IsWeaponNearBone();
    void UnSnapActor();
    bool SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName);

    // Rotation
    void SaveRotationData() const;
    void ResetHoldingData();
    void SetWeaponRotation(FQuat Rotation) const;
    void SetCurrentRotationSelectedActor(AActor* TargetActor);

    // Debug
    void DrawDebugHighlight() const;

public:
    AActor* CurrentSpawnedActor;
    TWeakObjectPtr<UTGCGameInstance> MyGameInstance;

private:
    TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset;
    TWeakObjectPtr<UTGWeaponDataAsset> WeaponDataAsset;

    // STORED VALUE
    TObjectPtr<class USkeletalMeshComponent> MySkeletalMeshComponent;
    AActor* CurrentRotationSelectedActor;
    FName CurrentTargetBone;

    // PROPERTIES
    UPROPERTY(EditAnywhere)
    float SnapCheckDistance = 50.0f;
};

// Forward declare UScrollBox
class UScrollBox;
