// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TGHpStatBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Interface/TGCharacterWidgetInterface.h"

UTGHpStatBarWidget::UTGHpStatBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MaxHp = -1.0f;
}

void UTGHpStatBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	HpProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PbHpBar")));
	ensure(HpProgressBar);
	
	HpText = Cast<UTextBlock>(GetWidgetFromName(TEXT("PbHpText")));
	ensure(HpText);
	


	ITGCharacterWidgetInterface* CharacterWidget = Cast<ITGCharacterWidgetInterface>(OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetupCharacterWidget(this);
	}

	
}

void UTGHpStatBarWidget::UpdateHpBar(float NewCurrentHp)
{
	ensure(MaxHp > 0.0f);
	MaxHp = NewCurrentHp > MaxHp ? NewCurrentHp : MaxHp;

	if(HpProgressBar)
	{
		HpProgressBar->SetPercent(NewCurrentHp/MaxHp);
	}
	if (HpText)
	{
		HpText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), FMath::FloorToInt(NewCurrentHp), FMath::FloorToInt(MaxHp))));
	}
}

