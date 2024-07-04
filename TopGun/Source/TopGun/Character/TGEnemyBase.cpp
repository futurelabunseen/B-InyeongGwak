#include "TGEnemyBase.h"
#include "TGCharacterPlayer.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/TGCharacterStatComponent.h"

ATGEnemyBase::ATGEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("CPROFILE_TGCAPSULE"));
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	Stat->SetHp(50);
	ContactDamage = 25.0f;
	KnockBackAmount = 30;
}

void ATGEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	Stat->SetMaxHp(Health);
	MyAIController = Cast<AAIController>(GetController());
}

void ATGEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATGEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ATGEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ATGEnemyBase::Die()
{
	OnDeath.Broadcast();
	SetLifeSpan(0.05f);
	DeinitializeMonster();
}

void ATGEnemyBase::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Other)
	{
		FDamageEvent DamageEvent;
		if (ATGCharacterPlayer* PlayerActor = Cast<ATGCharacterPlayer>(Other))
		{
			Other->TakeDamage(ContactDamage, DamageEvent, GetWorld()->GetFirstPlayerController(), this);
		}
	}
}

void ATGEnemyBase::InitializeMonster()
{
	UE_LOG(LogTemp, Log, TEXT("Monster Initialized"));
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	RegisterDeathDelegate();
}

void ATGEnemyBase::DeinitializeMonster()
{
	UE_LOG(LogTemp, Log, TEXT("Monster Deinitialized"));
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	UnregisterDeathDelegate();
}

void ATGEnemyBase::RegisterDeathDelegate()
{
	if (UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		OnDeath.AddDynamic(GameInstance, &UTGCGameInstance::OnMonsterDeath);
	}
}

void ATGEnemyBase::UnregisterDeathDelegate()
{
	if (UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		OnDeath.RemoveDynamic(GameInstance, &UTGCGameInstance::OnMonsterDeath);
	}
}

void ATGEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAllTimers();
	Super::EndPlay(EndPlayReason);
}

void ATGEnemyBase::ClearAllTimers()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
}
