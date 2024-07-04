// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/TGCharacterStatComponent.h"

#include "GameInstance/TGGameInstance.h"
#include "Kismet/GameplayStatics.h"

UTGCharacterStatComponent::UTGCharacterStatComponent()
{
	MaxHp = 0.0f;
	CurrentHp = MaxHp;
}


// Called when the game starts
void UTGCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

float UTGCharacterStatComponent::ApplyDamage(float InDamage)
{
	const float PrevHp = CurrentHp;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, InDamage);

	SetHp(PrevHp - ActualDamage);
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();

		TWeakObjectPtr<UTGCGameInstance> MyGameInstance = Cast<UTGCGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

		if (MyGameInstance.IsValid())
		{
			OnZeroScore.Broadcast(MyGameInstance->PlayerScore);
			MyGameInstance->PlayerScore = 0;
		}
	}

	return ActualDamage;
}

void UTGCharacterStatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, MaxHp);
	OnHpChanged.Broadcast(CurrentHp);
}

void UTGCharacterStatComponent::SetMaxHp(float NewHp)
{
	MaxHp = NewHp;
	CurrentHp = MaxHp;
	OnHpChanged.Broadcast(CurrentHp);
}


