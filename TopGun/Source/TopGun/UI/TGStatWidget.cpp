// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/TGStatWidget.h"
#include "Components/TextBlock.h"
#include "Interface/TGCharacterWidgetInterface.h"
#include "Engine/Engine.h" // For GEngine and UE_LOG

UTGStatWidget::UTGStatWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UTGStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AttackText = Cast<UTextBlock>(GetWidgetFromName(TEXT("PbAttackNum")));
	if (AttackText)
	{
		UE_LOG(LogTemp, Log, TEXT("AttackText successfully found and cast."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackText could not be found or cast."));
	}
	
	HpMaxText = Cast<UTextBlock>(GetWidgetFromName(TEXT("PbDefenseNum")));
	if (HpMaxText)
	{
		UE_LOG(LogTemp, Log, TEXT("HpMaxText successfully found and cast."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HpMaxText could not be found or cast."));
	}
	

	
	ITGCharacterWidgetInterface* CharacterWidget = Cast<ITGCharacterWidgetInterface>(OwningActor);
	if (CharacterWidget)
	{
		CharacterWidget->SetupCharacterWidget(this);
	}

	
}

void UTGStatWidget::UpdateStatBar(int maxhp, int attack)
{
	if (AttackText)
	{
		UE_LOG(LogTemp, Log, TEXT("Updating AttackText with value: %d"), attack);
		AttackText->SetText(FText::FromString(FString::Printf(TEXT("Health :  %d"), attack)));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackText is null. Cannot update AttackText."));
	}

	if (HpMaxText)
	{
		UE_LOG(LogTemp, Log, TEXT("Updating HpMaxText with value: %d"), maxhp);
		HpMaxText->SetText(FText::FromString(FString::Printf(TEXT("Attack : %d"), maxhp)));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HpMaxText is null. Cannot update HpMaxText."));
	}
}
