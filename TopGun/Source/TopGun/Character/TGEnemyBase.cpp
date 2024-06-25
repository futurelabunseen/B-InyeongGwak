#include "TGEnemyBase.h"

#include "TGCharacterPlayer.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Utility/TGCharacterStatComponent.h"

ATGEnemyBase::ATGEnemyBase()
{
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("CPROFILE_TGCAPSULE"));
    GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
    MyAIController = Cast<AAIController>(GetController());
    
    PrimaryActorTick.bCanEverTick = true;
    Stat->SetHp(50);
    ContactDamage = 10.0f;
    KnockBackAmount = 15;
}

void ATGEnemyBase::BeginPlay()
{
    Super::BeginPlay();
}

void ATGEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATGEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

/*
void ATGEnemyBase::Die()
{
    OnDeath.Broadcast();
    GetMesh()->SetSimulatePhysics(true);
    DropItem();
    SetLifeSpan(0.25f); 
}

void ATGEnemyBase::DropItem()
{

}
*/


void ATGEnemyBase::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
    bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    UE_LOG(LogTemp, Warning, TEXT("NotifyHit from Enemy"));
    FDamageEvent DamageEvent;
    Other->TakeDamage(ContactDamage, DamageEvent, GetWorld()->GetFirstPlayerController(), this);
}

