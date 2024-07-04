// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/TGUserWidget.h"
#include "TGStatWidget.generated.h"

/**
 * 
 */
UCLASS()
class TOPGUN_API UTGStatWidget : public UTGUserWidget
{
	GENERATED_BODY()
public:
	UTGStatWidget(const FObjectInitializer& ObjectInitializer);
	void UpdateStatBar(int maxhp, int attack);

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY()
	TObjectPtr<class UTextBlock> AttackText;

	UPROPERTY()
	TObjectPtr<class UTextBlock> HpMaxText;
};
