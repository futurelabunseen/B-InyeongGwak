// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Utility/TGCustomizingStateManager.h" 
#include "Utility/TGCustomizingComponent.h" 
#include "Utility/TGCustomizingUIManager.h"
#include "TGCustomizationGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TOPGUN_API ATGCustomizationGameMode : public AGameModeBase
{
	GENERATED_BODY()
	ATGCustomizationGameMode();

public:
	UPROPERTY()
	UTGCustomizingStateManager* StateManager;
	UPROPERTY()
	UTGCustomizationHandlingManager* CustomizingComponent;
	UPROPERTY()
	UTGCustomizingUIManager* CustomizingUIManager;
	
	ITGCustomizingPlayerInterface* CustomizingPlayerInterface;
	ITGCustomizingInterface* CustomizingStateInterface;
	void PostLogin(APlayerController* NewPlayer);
};
