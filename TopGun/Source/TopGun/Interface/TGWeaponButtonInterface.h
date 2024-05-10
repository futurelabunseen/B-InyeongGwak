// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TGWeaponButtonInterface.generated.h"

DECLARE_DELEGATE_TwoParams(FWeaponButtonClickedDelegate, FName , UObject* );

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTGWeaponButtonInterface : public UInterface
{
	GENERATED_BODY()
};

class TOPGUN_API ITGWeaponButtonInterface
{
	GENERATED_BODY()

public:
    virtual void SetupButton(FName WeaponName, FWeaponButtonClickedDelegate ClickedDelegate) = 0;
};