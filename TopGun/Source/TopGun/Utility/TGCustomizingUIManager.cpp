// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/TGCustomizingUIManager.h"

#include "TGCustomizationHandlingManager.h"
#include "TGEquipmentManager.h"
#include "TGModuleDataAsset.h"
#include "Components/ScrollBox.h"
#include "GameInstance/TGGameInstance.h"
#include "UI/TGEquipWidget.h"
#include "UI/TGInventoryWeaponButton.h"

UTGCustomizingUIManager::UTGCustomizingUIManager()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> EquipButtonWidgetClassFinder(TEXT("/Game/TopGun/Blueprint/Widget/BP_EquipWidget.BP_EquipWidget_C"));
	if (EquipButtonWidgetClassFinder.Succeeded())
	{
		EquipButtonWidgetClass = EquipButtonWidgetClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> ModuleButtonWidgetClassFinder(TEXT("/Game/TopGun/Blueprint/Widget/BP_InveModuleButton.BP_InveModuleButton_C"));
	if (ModuleButtonWidgetClassFinder.Succeeded())
	{
		ModuleButtonWidgetClass = ModuleButtonWidgetClassFinder.Class;
	}

	
}

void UTGCustomizingUIManager::Initialize(const TWeakObjectPtr<UTGCustomizationHandlingManager> InCustomizingComponent, UWorld* World)
{
	MyCustomizingComponent = InCustomizingComponent;
	if (!MyCustomizingComponent->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("UTGCustomizingUIManager::Initialize - CustomizingComponent is null"));
	}
	MyGameInstance = Cast<UTGCGameInstance>(World->GetGameInstance());
	if (!MyGameInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UIManager INIt::Failed to cast GameInstance to UTGCGameInstance."));
		return;
	}
	MyGameInstance = Cast<UTGCGameInstance>(World->GetGameInstance());
	if (!MyGameInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("TGCUstomizingUIManager::Init::Failed to cast GameInstance to UTGCGameInstance."));
		return;
	}
}

void UTGCustomizingUIManager::AddWeaponButtonToPanel(UScrollBox* TargetPanel)
{
	if (!TargetPanel || !EquipButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters for GenerateEquipButtons"));
		return;
	}

	TArray<TPair<FName, FEquipmentData>> EquipmentDataArray = MyGameInstance->GetEquipmentManager()->GetEquipmentDataForCategory(ETGEquipmentCategory::Weapon);

	for (const auto& Pair : EquipmentDataArray)
	{
		GenerateEquipButtonProcessEquipRow(Pair.Key, TargetPanel);
	}
}

void UTGCustomizingUIManager::AddArmourButtonToPanel(UScrollBox* TargetPanel)
{
	if (!TargetPanel || !EquipButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters for GenerateEquipButtons"));
		return;
	}

	TArray<TPair<FName, FEquipmentData>> EquipmentDataArray = MyGameInstance->GetEquipmentManager()->GetEquipmentDataForCategory(ETGEquipmentCategory::Armour);

	for (const auto& Pair : EquipmentDataArray)
	{
		GenerateEquipButtonProcessEquipRow(Pair.Key, TargetPanel);
	}
}

void UTGCustomizingUIManager::GenerateEquipButtonProcessEquipRow(const FName& Key, UScrollBox* TargetPanel) const
{
	UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(GetWorld(), EquipButtonWidgetClass);
	if (!CreatedWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create widget"));
		return;
	}

	if (auto* InventoryButton = Cast<UTGEquipWidget>(CreatedWidget))
	{
		InventoryButton->SetupButton(Key, MyGameInstance->GetEquipmentManager()->GetEquipPointsByID(Key));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryEquipButton"));
		return;
	}

	TargetPanel->AddChild(CreatedWidget);
}

void UTGCustomizingUIManager::AddModuleButtonToPanel(UScrollBox* TargetPanel)
{
	if (!TargetPanel || !ModuleButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid parameters. Module"));
		return;
	}

	for (const TPair<FName, FMeshCategoryData>& TargetMap : MyGameInstance->ModuleDataAsset->BaseMeshComponent)
	{
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(GetWorld(), ModuleButtonWidgetClass);
		if (!CreatedWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Created widget null"));
			return;
		}
		if (UTGInventoryWeaponButton* InventoryButton = Cast<UTGInventoryWeaponButton>(CreatedWidget))
		{
			InventoryButton->SetupButton(TargetMap.Key, 0, 0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to cast the created widget to TGInventoryWeaponButton."));
			return;
		}
		TargetPanel->AddChild(CreatedWidget);
	}
}



void UTGCustomizingUIManager::OnModuleSelected(FName WeaponID, APlayerController* Player)
{
	if (MyCustomizingComponent->IsValidLowLevel())
	{
		MyCustomizingComponent->AlterModuleComponent(WeaponID, Player);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CustomizingComponent is not valid."));
	}
}

void UTGCustomizingUIManager::RegisterWeaponSelectButton(UUserWidget* TargetWidget)
{
	CurrentWeaponToolWidget = TargetWidget;
	if(CurrentWeaponToolWidget)
		CurrentWeaponToolWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UTGCustomizingUIManager::ToggleCurrentWeaponToolWidget(bool value) const
{
	if(CurrentWeaponToolWidget)
			CurrentWeaponToolWidget->SetVisibility(value==true?ESlateVisibility::Visible:ESlateVisibility::Hidden);
}

