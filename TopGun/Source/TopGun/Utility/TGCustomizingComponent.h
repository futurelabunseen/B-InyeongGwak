#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ScrollBox.h"
#include "Data/EquipmentData.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWidget")
    TSubclassOf<UUserWidget> ArmourButtonWidgetClass;

    void GenerateModuleButtons(UScrollBox* TargetPanel) const;

    // Customizing Features
    static USkeletalMesh* GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset);
    void GenerateEquipButtonProcessEquipRow(const FName& Key, UScrollBox* TargetPanel) const;
    void GenerateEquipButtons(UScrollBox* TargetPanel, ETGEquipmentCategory category) const;

    // Spawn
    AActor* SpawnEquip(FName EquipID);
    void SpawnCurrentEquip(FName EquipID);
    void SpawnModule(FName WeaponID) const; // MeshMerge Method1
    void AlterModuleComponent(FName WeaponID); // LeaderPoseComponent Method2
    bool AttachActor() const;
    bool EquipRegister(AActor* ClonedActor) const;

    // Handling Spawned Actor
    void RemoveEquipFromCharacter(AActor* EquipToRemove) const;
    void UpdateWeaponActorPosition(const FVector& WorldLocation, const FVector& WorldDirection) const;
    bool IsEquipNearBone();
    bool IsWithinSnapDistance(float distance, const FVector& boneLocation, FName boneName);
    void UnSnapActor();
    bool SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName);
    void HighlightSelectedActor(bool bEnable);

    
    // Rotation
    void SaveRotationData() const;
    void ResetHoldingData();
    void SetTargetActorRotation(FQuat Rotation) const;
    bool SetCurrentSelectedActor(AActor* TargetActor);
    AActor* GetCurrentSelectedActor() const;

    // Debug
    void DrawDebugHighlight() const;

public:
    AActor* CurrentSpawnedActor;
    TWeakObjectPtr<UTGCGameInstance> MyGameInstance;
    
    
private:
    TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset;

    // STORED VALUE
    TObjectPtr<class USkeletalMeshComponent> MySkeletalMeshComponent;
    AActor* CurrentSelectedActor;
    FName CurrentTargetBone;
    bool bIsHighlighted;

    // PROPERTIES
    UPROPERTY(EditAnywhere)
    float SnapCheckDistance = 50.0f;
};

// Forward declare UScrollBox
class UScrollBox;
