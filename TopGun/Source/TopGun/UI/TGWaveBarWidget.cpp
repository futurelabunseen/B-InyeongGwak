// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TGWaveBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Interface/TGCharacterWidgetInterface.h"

UTGWaveBarWidget::UTGWaveBarWidget(const FObjectInitializer& ObjectInitializer)
{
}

void UTGWaveBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WaveProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PbProgressBar")));
	ensure(WaveProgressBar);

	WaveLevelText = Cast<UTextBlock>(GetWidgetFromName(TEXT("PbLevelNum")));
	ensure(WaveLevelText);

	ScoreText = Cast<UTextBlock>(GetWidgetFromName(TEXT("PbScoreText")));
	ensure(ScoreText);
	
	
	
	ITGCharacterWidgetInterface* CharacterWidget = Cast<ITGCharacterWidgetInterface>(OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetupCharacterWidget(this);
	}
}

void UTGWaveBarWidget::UpdateWaveBar(int level, float NewPercentage, int score)
{
	if (WaveProgressBar)
	{
		WaveProgressBar->SetPercent(NewPercentage);
	}

	if (WaveLevelText)
	{
		WaveLevelText->SetText(FText::FromString(FString::Printf(TEXT("Wave %d"), level)));
	}

	if (WaveLevelText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("SCORE : %d"), score)));
	}
}