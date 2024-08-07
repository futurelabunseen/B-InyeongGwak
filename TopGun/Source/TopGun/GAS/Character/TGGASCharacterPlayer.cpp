// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Character/TGGASCharacterPlayer.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/Player/TGGASPlayerState.h"
ATGGASCharacterPlayer::ATGGASCharacterPlayer()
{
	ASC = nullptr; //There should be only one ASC for player
}

UAbilitySystemComponent* ATGGASCharacterPlayer::GetAbilitySystemComponent() const
{
	return ASC;
}

void ATGGASCharacterPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ATGGASPlayerState* GASPS = GetPlayerState<ATGGASPlayerState>();
	if (GASPS)
	{
		ASC = GASPS->GetAbilitySystemComponent();
		ASC->InitAbilityActorInfo(GASPS, this);
		int32 InputId = 0;
		
		for (const auto& StartAbility : StartAbilities)
		{
			FGameplayAbilitySpec StartSpec(StartAbility);
			StartSpec.InputID = InputId++; //Set the ability's ID and store it in a SPEC
			ASC->GiveAbility(StartSpec);
		}
	}
}

void ATGGASCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	SetupGASInputComponent();
}

void ATGGASCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void ATGGASCharacterPlayer::SetupGASInputComponent()
{
	if(IsValid(ASC) && IsValid(InputComponent))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ATGGASCharacterPlayer::GASInputPressed,0); //zereo is the input ID
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATGGASCharacterPlayer::GASInputReleased,0);
	}
}

void ATGGASCharacterPlayer::GASInputPressed(int32 InputId) //parameter is input info(ID)
{
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(InputId);
	//Find the ability in ASC via inputID and trigger the ability
	if (Spec)
	{
		Spec->InputPressed = true;
		if(Spec->IsActive()) //Check if the Ability is already being played
		{
			ASC->AbilitySpecInputPressed(*Spec); //For the Spec info trigger input pressed
		} else
		{
			ASC->TryActivateAbility(Spec->Handle); //Give Handle and activate
		}
	}
}

void ATGGASCharacterPlayer::GASInputReleased(int32 InputId)
{	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(InputId);
	if (Spec)
	{
		Spec->InputPressed = false;
		if(Spec->IsActive())
		{
			ASC->AbilitySpecInputReleased(*Spec); 
		} 
	}
}

