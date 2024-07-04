// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TGGameOverWidget.h"

#include "Components/TextBlock.h"

UTGGameOverWidget::UTGGameOverWidget(const FObjectInitializer& ObjectInitializer)
{
}

void UTGGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ScoreText = Cast<UTextBlock>(GetWidgetFromName(TEXT("ScoreText")));
	ensure(ScoreText);
	this->SetVisibility(ESlateVisibility::Collapsed);
}

void UTGGameOverWidget::UpdateScore(int score)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Your Score %d"), score)));
		this->SetVisibility(ESlateVisibility::Visible);
	}
}
