// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TGEquipWidget.h"

#include "Components/TextBlock.h"

void UTGEquipWidget::SetupButton(FName EquipName)
{
	if (TitleText)
	{
		TitleText->SetText(FText::FromName(EquipName));
		EquipID = EquipName;
	}
}
