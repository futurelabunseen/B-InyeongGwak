// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TGCharacterBase.h"

#include "TGPlayerControlData.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/TGHpStatBarWidget.h"
#include "UI/TGWidgetComponent.h"
#include "Utility/TGCharacterStatComponent.h"

// Sets default values
ATGCharacterBase::ATGCharacterBase()
{
	// Pawn
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("CPROFILE_TGCAPSULE"));
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	Stat = CreateDefaultSubobject<UTGCharacterStatComponent>(TEXT("Stat"));
	HpBar = CreateDefaultSubobject<UTGWidgetComponent>(TEXT("Widget"));
	HpBar->SetupAttachment(GetMesh());
	HpBar->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	static ConstructorHelpers::FClassFinder<UUserWidget> HpBarWidgetRef(TEXT("/Game/TopGun/Blueprint/Widget/HPStatBar.HPStatBar_C"));
	if (HpBarWidgetRef.Class)
	{
		HpBar->SetWidgetClass(HpBarWidgetRef.Class);
		HpBar->SetWidgetSpace(EWidgetSpace::Screen);
		HpBar->SetDrawSize(FVector2D(150.0f, 0.0f));
		HpBar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

float ATGCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("%s DAMANGED HP by %s : %f"), *DamageCauser->GetActorNameOrLabel(), *GetActorNameOrLabel(),Stat->GetCurrentHp());
	if(Stat->GetCurrentHp() > 0)
	{
		FVector KnockbackDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
		LaunchCharacter(KnockbackDirection * KnockBackAmount, true, true);
	}
	Stat->ApplyDamage(DamageAmount);
	
	return DamageAmount;
}

void ATGCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Stat->OnHpZero.AddUObject(this, &ATGCharacterBase::Die);
}


void ATGCharacterBase::SetCharacterControlData(const UTGPlayerControlData* CharacterControlData)
{
	// Pawn
	bUseControllerRotationYaw = CharacterControlData->bUseControllerRotationYaw;
	// CharacterMovement
	GetCharacterMovement()->bOrientRotationToMovement = CharacterControlData->bOrientRotationToMovement;
	GetCharacterMovement()->bUseControllerDesiredRotation = CharacterControlData->bUseControllerDesiredRotation;
	GetCharacterMovement()->RotationRate = CharacterControlData->RotationRate;
}

void ATGCharacterBase::Die()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	SetActorEnableCollision(false);
	if (HpBar)
	{
		HpBar->SetHiddenInGame(true);
	}
	UE_LOG(LogTemp, Warning, TEXT("Object Died : %s"), *GetActorNameOrLabel());
}



void ATGCharacterBase::SetupCharacterWidget(UTGUserWidget* InUserWidget)
{
	UTGHpStatBarWidget* HpBarWidget = Cast<UTGHpStatBarWidget>(InUserWidget);
	if (HpBarWidget)
	{
		HpBarWidget->SetMaxHP(Stat->GetMaxHp());
		HpBarWidget->UpdateHpBar(Stat->GetCurrentHp());
		Stat->OnHpChanged.AddUObject(HpBarWidget, &UTGHpStatBarWidget::UpdateHpBar);
	}
}




