// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TGCustomizingStateManager.generated.h"

/**
 * 
 */



UCLASS()
class TOPGUN_API UTGCustomizingStateManager : public UObject
{
	GENERATED_BODY()
public:
	UTGCustomizingStateManager();
/*
	void Initialize(ATGCustomizingPlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void UpdateState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void EnterIdleState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void EnterDragState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void EnterSnappedState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void EnterRotateState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void EnterSelectActorState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void EnterBindKeyState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void ReturnToIdleState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	void ReturnToSelectActorState();

	UFUNCTION(BlueprintCallable, Category = "Customizing")
	ECustomizingState GetCurrentState() const { return CurrentState; }

private:
	void HandleIdleState();
	void HandleDragState();
	void HandleSnappedState();
	void HandleRotateState();
	void HandleSelectActorState();
	void HandleBindKeyState();

	UPROPERTY()
	ECustomizingState CurrentState;

	UPROPERTY()
	ATGCustomizingPlayerController* PlayerController;
	*/
};
