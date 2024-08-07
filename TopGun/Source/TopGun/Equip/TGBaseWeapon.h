#pragma once

#include "CoreMinimal.h"
#include "Equip/TGBaseEquipment.h"
#include "Interface/TGWeaponInterface.h"
#include "TGBaseWeapon.generated.h"

UCLASS()
class TOPGUN_API ATGBaseWeapon : public ATGBaseEquipment, public ITGWeaponInterface
{
	GENERATED_BODY()

public:
	ATGBaseWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	int32 Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FQuat DefaultRotationQuat;

	virtual void InitializeWeapon(FName WeaponID, FName BoneID);

	UFUNCTION(BlueprintCallable)
	void CheckForHitScan(bool bIsAiming);
//WEAPON
	virtual void SetDefaultRotation_Implementation() override;
	virtual void SetRotation_Implementation(const FQuat& QuatRotation) override;
	virtual FVector GetArrowForwardVector_Implementation() const override;
	virtual FQuat GetAimingRotation_Implementation(const FVector& TargetVector) const override;
//EQUIP
	virtual FName GetEquipmentID_Implementation() override;
	virtual FName GetBoneID_Implementation() override;
	virtual void SetEquipmentID_Implementation(FName NewEquipmentID) override;
	virtual void SetBoneID_Implementation(FName NewBoneID) override;
	virtual void InitializeEquipment_Implementation(FName NewEquipmentID, FName NewBoneID) override;
	virtual  USpringArmComponent* GetSpringArmComponent_Implementation() const override;
	virtual void SetSpringArmComponent_Implementation(USpringArmComponent* SpringArmComponent) override;
	virtual void BeginPlay() override;
	virtual ETGEquipmentCategory GetCategory_Implementation() override;

	
private:
	FQuat TargetRotationQuat;
	FQuat InitialForwardQuat;
};