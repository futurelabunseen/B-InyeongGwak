// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TGCharacterWidgetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTGCharacterWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native Interface Class
 */
class TOPGUN_API ITGCharacterWidgetInterface
{
	GENERATED_BODY()

	public:
	virtual void SetupCharacterWidget(class UTGUserWidget* InUserWidget) = 0;
};
