// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Player/TGGASPlayerState.h"
#include "AbilitySystemComponent.h"

ATGGASPlayerState::ATGGASPlayerState()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	//ASC -> SetIsReplicated(true);Multiplay GAS
}

UAbilitySystemComponent* ATGGASPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
	
}


