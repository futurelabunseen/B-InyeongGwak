// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TGEquipWidget.h"

#include "Components/TextBlock.h"

void UTGEquipWidget::SetupButton(FName EquipName, int32 PointProperty)
{
	if (TitleText)
	{
		TitleText->SetText(FText::FromName(EquipName));
		EquipID = EquipName;
	}

	PropertyText->SetText(FText::AsNumber(PointProperty));
}
