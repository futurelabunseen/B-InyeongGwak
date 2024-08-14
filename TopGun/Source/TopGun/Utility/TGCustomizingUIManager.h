// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInstance/TGGameInstance.h"
#include "TGCustomizingUIManager.generated.h"

/**
 * 
 */
UCLASS()
class TOPGUN_API UTGCustomizingUIManager : public UObject
{
	GENERATED_BODY()
	UTGCustomizingUIManager();

public:
	// UI

	
	UFUNCTION()
	void Initialize(const TWeakObjectPtr<UTGCustomizationHandlingManager> InCustomizingComponent, UWorld* World);
	UFUNCTION(BlueprintCallable)
	void AddWeaponButtonToPanel(UScrollBox* TargetPanel);
	UFUNCTION(BlueprintCallable)
	void AddModuleButtonToPanel(UScrollBox* TargetPanel);
	UFUNCTION(BlueprintCallable)
	void AddArmourButtonToPanel(UScrollBox* TargetPanel);
	UFUNCTION(BlueprintCallable)
	void OnModuleSelected(FName WeaponID, APlayerController* Player);
	UFUNCTION(BlueprintCallable)
	void RegisterWeaponSelectButton(UUserWidget* TargetWidget);
	UFUNCTION(BlueprintCallable)
	void GenerateEquipButtonProcessEquipRow(const FName& Key, UScrollBox* TargetPanel) const;


	void ToggleCurrentWeaponToolWidget(bool value) const;
private:
	TWeakObjectPtr<UTGCustomizationHandlingManager> MyCustomizingComponent;
	TSoftObjectPtr<UUserWidget> CurrentWeaponToolWidget;
	TSubclassOf<UUserWidget> ModuleButtonWidgetClass;
	TSubclassOf<UUserWidget> EquipButtonWidgetClass;
	TWeakObjectPtr<UTGCGameInstance> MyGameInstance;
};
