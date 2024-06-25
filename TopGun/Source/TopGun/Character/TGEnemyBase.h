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
	//virtual void Die() override;
public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeathDelegate OnDeath;

	//UFUNCTION()
	//void DropItem();
	

	UFUNCTION()
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float ContactDamage;

private:
	AAIController* MyAIController;
};
