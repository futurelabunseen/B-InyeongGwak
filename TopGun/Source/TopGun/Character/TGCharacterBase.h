// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TGCharacterBase.generated.h"


UCLASS()
class TOPGUN_API ATGCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ATGCharacterBase();
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


protected:
	virtual void SetCharacterControlData(const class UTGPlayerControlData* CharacterControlData);
	int Health;
	float KnockBackAmount;
	UFUNCTION()
	virtual void Die();
};
