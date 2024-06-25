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
	void CheckForHitScan(bool bIsAiming);

	void SetDefaultRotation();

	void SetRotation(const FQuat& QuatRotation);

	FVector GetArrowForwardVector() const;

	FQuat GetAimingRotation(const FVector& TargetVector) const;

//INTEFACE
	virtual void SetWeaponID_Implementation(FName WeaponID) override;
	virtual void SetBoneID_Implementation(FName BoneID) override;
	virtual FName GetWeaponID_Implementation() override;
	virtual FName GetBoneID_Implementation() override;
	
private:
	FQuat TargetRotationQuat;
	FQuat InitialForwardQuat;
};
