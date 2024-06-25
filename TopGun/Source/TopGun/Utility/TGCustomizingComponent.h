#pragma once

#include "CoreMinimal.h"
#include "TGWeaponDataAsset.h"
#include "Components/ActorComponent.h"
#include "Utility/TGModuleSystem.h"
#include "TGCustomizingComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
	static USkeletalMesh* GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset);
	static UBlueprintGeneratedClass* GetWeaponClassById(FName WeaponID, UTGWeaponDataAsset* WeaponDataAsset);
	void AddButtonToPanel(class UScrollBox* TargetPanel, TSubclassOf<class UUserWidget> TargetButtonWidget, FName ID) const;
	void AddWeaponButtonToPanel(UScrollBox* TargetPanel) const;
	void AddModuleButtonToPanel(UScrollBox* TargetPanel) const;
	AActor* SpawnWeapon(FName WeaponID);

	//MOVED
	void SpawnCurrentWeapon(FName WeaponID);
	void SpawnModule(FName WeaponID) const;
	bool AttachWeapon() const;
	void RemoveWeaponFromCharacter(AActor* WeaponToRemove) const;
	void UpdateWeaponActorPosition(const FVector& WorldLocation, const FVector& WorldDirection) const;
	bool IsWeaponNearBone();
	void UnSnapActor();
	bool SnapActor(FVector ClosestBoneLocation, float ClosestBoneDistance, FName ClosestBoneName);
	void SaveRotationData() const;
	void ResetHoldingData();
	void SetWeaponRotation(FQuat Rotation) const;
	void SetCurrentRotationSelectedActor(AActor* TargetActor);
	void DrawDebugHighlight() const;

public:
	AActor* CurrentSpawnedActor;
	AActor* CurrentSnappedActor;
	TWeakObjectPtr<UTGCGameInstance> MyGameInstance;

private:
	TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset;
	TWeakObjectPtr<UTGWeaponDataAsset> WeaponDataAsset;

//STOREDVALLUE
	TObjectPtr<class USkeletalMeshComponent> MySkeletalMeshComponent;
	AActor* CurrentRotationSelectedActor;
	FName CurrentTargetBone;

	//PROPERTIES
	UPROPERTY()
	float SnapCheckDistance = 50.0f;
};

// Forward declare UScrollBox
class UScrollBox;