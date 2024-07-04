// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TGUserWidget.h"
#include "TGWaveBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class TOPGUN_API UTGWaveBarWidget : public UTGUserWidget
{
	GENERATED_BODY()
public:
	UTGWaveBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	
public:
	void UpdateWaveBar(int level, float NewPercentage, int score);
protected:
	UPROPERTY()
	TObjectPtr<class UProgressBar> WaveProgressBar;
	UPROPERTY()
	TObjectPtr<class UTextBlock> WaveLevelText;
	UPROPERTY()
	TObjectPtr<class UTextBlock> ScoreText;
};
