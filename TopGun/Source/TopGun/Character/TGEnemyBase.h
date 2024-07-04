#pragma once

#include "CoreMinimal.h"
#include "TGCharacterBase.h"
#include "GameFramework/Character.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "TGEnemyBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);

UCLASS()
class TOPGUN_API ATGEnemyBase : public ATGCharacterBase
{
	GENERATED_BODY()

public:
	ATGEnemyBase();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Die() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Health = 0;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeathDelegate OnDeath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float ContactDamage;

	UFUNCTION()
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	void InitializeMonster();
	void DeinitializeMonster();

private:
	AAIController* MyAIController;
	void RegisterDeathDelegate();
	void UnregisterDeathDelegate();
	void ClearAllTimers();
};
