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

	virtual void InitializeWeapon(FName WeaponID, FName BoneID) override;

	UFUNCTION(BlueprintCallable)
	void CheckForHitScan(bool bIsAiming);

	virtual void SetDefaultRotation() override;

	virtual void SetRotation(const FQuat& QuatRotation) override;

	virtual FVector GetArrowForwardVector() const override;

	virtual FQuat GetAimingRotation(const FVector& TargetVector) const override;
//INTERFACE
	virtual void SetWeaponID_Implementation(FName WeaponID) override;
	virtual void SetBoneID_Implementation(FName BoneID) override;
	virtual FName GetWeaponID_Implementation() override;
	virtual FName GetBoneID_Implementation() override;
	virtual FQuat GetDefaultRoationQuat() const override;
	virtual void SetSpringArmComponent(USpringArmComponent* SpringArmComponent) override;
	virtual USpringArmComponent* GetSpringArmComponent() const override;
private:
	FQuat TargetRotationQuat;
	FQuat InitialForwardQuat;
};
