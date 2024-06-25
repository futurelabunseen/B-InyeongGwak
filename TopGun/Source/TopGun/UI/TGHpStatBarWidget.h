// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TGUserWidget.h"
#include "TGHpStatBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class TOPGUN_API UTGHpStatBarWidget : public UTGUserWidget
{
	GENERATED_BODY()

public:
	UTGHpStatBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	
public:
	FORCEINLINE void SetMaxHP(float NewMaxHP){ MaxHp = NewMaxHP; }
	void UpdateHpBar(float NewCurrentHp);
protected:
	UPROPERTY()
	TObjectPtr<class UProgressBar> HpProgressBar;

	UPROPERTY()
	float MaxHp;
};
