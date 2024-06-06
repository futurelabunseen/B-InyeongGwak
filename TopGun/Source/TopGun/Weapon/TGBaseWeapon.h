#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/TGWeaponInterface.h"
#include "GameFramework/SpringArmComponent.h"
#include "TGBaseWeapon.generated.h"

UCLASS()
class TOPGUN_API ATGBaseWeapon : public AActor, public ITGWeaponInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATGBaseWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName BoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FQuat DefaultRotationQuat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USpringArmComponent* MySpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AttackDamage;

	void InitializeWeapon(FName WeaponID, FName BoneID);

	UFUNCTION(BlueprintCallable)
	void CheckForHitScan();

	void SetDefaultRotation();

	void SetRotation(const FQuat& QuatRotation);

	FVector GetArrowForwardVector() const;

	FQuat GetAimingRotation(const FVector& TargetVector) const;

private:
	FQuat TargetRotationQuat;
	FQuat InitialForwardQuat;
};
