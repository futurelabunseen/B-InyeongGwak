// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "TGGASPlayerState.generated.h"
/**
 * 
 */
UCLASS()
class TOPGUN_API ATGGASPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATGGASPlayerState();
	
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected :
	UPROPERTY(EditAnywhere, Category = GAS)
	TObjectPtr<class UAbilitySystemComponent> ASC;

	
};
