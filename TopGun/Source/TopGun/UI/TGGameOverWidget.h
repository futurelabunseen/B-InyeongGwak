// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/TGUserWidget.h"
#include "TGGameOverWidget.generated.h"

/**
 * 
 */
UCLASS()
class TOPGUN_API UTGGameOverWidget : public UTGUserWidget
{
	GENERATED_BODY()
public:
	UTGGameOverWidget(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void NativeConstruct() override;
	
public:
	void UpdateScore(int score);
protected:
	UPROPERTY()
	TObjectPtr<class UTextBlock> ScoreText;
};
