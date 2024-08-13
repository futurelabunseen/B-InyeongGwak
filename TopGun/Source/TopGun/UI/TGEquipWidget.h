// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "TGEquipWidget.generated.h"

/**
 * 
 */
UCLASS()
class TOPGUN_API UTGEquipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetupButton(FName EquipName, int32 PointProperty);
	UPROPERTY(BlueprintReadOnly)
	FName EquipID;
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PropertyText;
};
