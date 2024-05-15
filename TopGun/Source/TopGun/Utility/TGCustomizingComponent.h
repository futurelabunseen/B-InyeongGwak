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
	void AddWeaponButtonToPanel(UScrollBox* TargetPanel, TWeakObjectPtr<UTGWeaponDataAsset> WeaponDataAsset);
	void AddModuleButtonToPanel(UScrollBox* TargetPanel, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset);
};

// Forward declare UScrollBox
class UScrollBox;