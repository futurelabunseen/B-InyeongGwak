// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TGInventoryWeaponButton.h"

void UTGInventoryWeaponButton::SetupButton(FName WeaponName, int32 attackProperty, int32 defenseProperty)
{
	if (TitleText)
	{
		TitleText->SetText(FText::FromName(WeaponName));
		WeaponID = WeaponName;
	}

	if (attackProperty>0)
	{
		PropertyText->SetText(FText::AsNumber(attackProperty));
	}

	if (defenseProperty>0)
	{
		PropertyText->SetText(FText::AsNumber(defenseProperty));
	}
}