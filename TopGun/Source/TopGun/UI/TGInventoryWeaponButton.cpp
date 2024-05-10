// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TGInventoryWeaponButton.h"

void UTGInventoryWeaponButton::SetupButton(FName WeaponName)
{
	if (TitleText)
	{
		TitleText->SetText(FText::FromName(WeaponName));
		WeaponID = WeaponName;
	}
}