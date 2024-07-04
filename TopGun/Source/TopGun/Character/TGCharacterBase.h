// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/TGCharacterWidgetInterface.h"
#include "TGCharacterBase.generated.h"


UCLASS()
class TOPGUN_API ATGCharacterBase : public ACharacter, public ITGCharacterWidgetInterface
{
	GENERATED_BODY()

public:
	ATGCharacterBase();
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void PostInitializeComponents() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UTGCharacterStatComponent> Stat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<UParticleSystem*> DamageEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<UParticleSystem*> DeathEffect;

protected:
	virtual void SetCharacterControlData(const class UTGPlayerControlData* CharacterControlData);
	float KnockBackAmount;
	UFUNCTION()
	virtual void Die();


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widget, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWidgetComponent> HpBar;

	UFUNCTION(BlueprintCallable)
	virtual void SetupCharacterWidget(class UTGUserWidget* InUserWidget) override;
	
	void PlayEffect(TArray<UParticleSystem*> Effect, float Time);
};
