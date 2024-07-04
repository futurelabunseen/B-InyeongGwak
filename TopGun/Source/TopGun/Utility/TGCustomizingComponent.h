#pragma once

#include "CoreMinimal.h"
#include "TGArmoursDataAsset.h"
#include "TGWeaponDataAsset.h"
#include "Components/ActorComponent.h"
#include "Components/ScrollBox.h"
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

    void GenerateWeaponButtons(UScrollBox* TargetPanel) const;
    void GenerateModuleButtons(UScrollBox* TargetPanel) const;
    void GenerateArmourButtons(UScrollBox* TargetPanel) const;

    // Customizing Features
    static USkeletalMesh* GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset);
    static UBlueprintGeneratedClass* GetWeaponClassById(FName WeaponID, UTGWeaponDataAsset* WeaponDataAsset);
    static UBlueprintGeneratedClass* GetArmourClassById(FName ArmourID, UTGArmoursDataAsset* ArmourDataAsset);
   

    // Spawn
    AActor* SpawnWeapon(FName WeaponID);
    AActor* SpawnArmour(FName ArmourID);
    void SpawnCurrentWeapon(FName WeaponID);
    void SpawnCurrentArmour(FName ArmourID);
    void SpawnModule(FName WeaponID) const; // MeshMerge Method1
    void AlterModuleComponent(FName WeaponID); // LeaderPoseComponent Method2
    bool AttachActor() const;
    bool WeaponRegister(AActor* ClonedActor) const;
    bool ArmourRegister(AActor* ClonedActor) const;

    // Handling Spawned Actor
    void RemoveWeaponFromCharacter(AActor* WeaponToRemove) const;
    void RemoveArmourFromCharacter(AActor* ArmourToRemove, FName TempBoneID) const;
    void UpdateWeaponActorPosition(const FVector& WorldLocation, const FVector& WorldDirection) const;

    bool IsWeaponNearBone();
    bool IsValidForWeaponCheck() const;
    bool IsWithinSnapDistance(float distance, const FVector& boneLocation, FName boneName);
    void UnSnapActor();
    bool SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName);

    // Rotation
    void SaveRotationData() const;
    void ResetHoldingData();
    void SetTargetActorRotation(FQuat Rotation) const;
    bool SetCurrentRotationSelectedActor(AActor* TargetActor);

    // Debug
    void DrawDebugHighlight() const;

public:
    AActor* CurrentSpawnedActor;
    TWeakObjectPtr<UTGCGameInstance> MyGameInstance;

private:
    TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset;
    TWeakObjectPtr<UTGWeaponDataAsset> WeaponDataAsset;
    TWeakObjectPtr<UTGArmoursDataAsset> ArmourDataAsset;

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
