#include "TGCustomizingComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "GameInstance/TGGameInstance.h"
#include "UI/TGInventoryWeaponButton.h"
#include "Utility/TGModuleSystem.h"
#include "Utility/TGModuleDataAsset.h"
#include "Utility/TGWeaponDataAsset.h"
#include "GameFramework/Actor.h"


UTGCustomizingComponent::UTGCustomizingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTGCustomizingComponent::BeginPlay()
{
	Super::BeginPlay();
}

USkeletalMesh* UTGCustomizingComponent::GetMergedCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset)
{
	return UTGModuleSystem::GetMergeCharacterParts(WholeModuleData, ModuleDataAsset.Get());
}

UBlueprintGeneratedClass* UTGCustomizingComponent::GetWeaponClassById(FName WeaponID, class UTGWeaponDataAsset* WeaponDataAsset)
{
	if (!WeaponDataAsset) return nullptr;
	UBlueprintGeneratedClass** FoundWeaponClass = WeaponDataAsset->BaseWeaponClasses.Find(WeaponID);
	return FoundWeaponClass ? *FoundWeaponClass : nullptr;
}

void UTGCustomizingComponent::AddButtonToPanel(UScrollBox* TargetPanel, TSubclassOf<UUserWidget> TargetButtonWidget, const FName ID) const
{
	if (!GetOwner() || !TargetPanel || !TargetButtonWidget) return;
	
	UWorld* World = GetOwner()->GetWorld();
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(World, TargetButtonWidget);
	if (!CreatedWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Created widget null"));
		return;
	}
	UTGInventoryWeaponButton* InventoryButton = Cast<UTGInventoryWeaponButton>(CreatedWidget);
	if (InventoryButton)
	{
		InventoryButton->SetupButton(ID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryWeaponButton."));
		return;
	}
	TargetPanel->AddChild(CreatedWidget);
}

void UTGCustomizingComponent::AddWeaponButtonToPanel(UScrollBox* TargetPanel, TWeakObjectPtr<UTGWeaponDataAsset> WeaponDataAsset)
{
	if (!TargetPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddWeaponButtonToPanel: TargetPanel is null."));
		return;
	}

	if (!WeaponButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddWeaponButtonToPanel: WeaponButtonWidgetClass is null."));
		return;
	}

	if (!WeaponDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddWeaponButtonToPanel: WeaponDataAsset is null."));
		return;
	}


	for (const TPair<FName, UBlueprintGeneratedClass*>& WeaponPair : WeaponDataAsset->BaseWeaponClasses)
	{
		AddButtonToPanel(TargetPanel, WeaponButtonWidgetClass, WeaponPair.Key);
	}
}

void UTGCustomizingComponent::AddModuleButtonToPanel(UScrollBox* TargetPanel,  TWeakObjectPtr<UTGModuleDataAsset> ModuleDataAsset)
{
	if (!TargetPanel || !ModuleButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters. Module"));
		return;
	}

	for (const TPair<FName, FMeshCategoryData>& TargetMap : ModuleDataAsset->BaseMeshComponent)
	{
		AddButtonToPanel(TargetPanel, ModuleButtonWidgetClass, TargetMap.Key);
	}
}
