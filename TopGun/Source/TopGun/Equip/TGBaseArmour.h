#pragma once

#include "CoreMinimal.h"
#include "Equip/TGBaseEquipment.h"
#include "Interface/TGArmourInterface.h"
#include "TGBaseArmour.generated.h"

UCLASS()
class TOPGUN_API ATGBaseArmour : public ATGBaseEquipment, public ITGArmourInterface
{
	GENERATED_BODY()

public:
	ATGBaseArmour();
	
	//EQUIP
	virtual FName GetEquipmentID_Implementation() override;
	virtual FName GetBoneID_Implementation() override;
	virtual void SetEquipmentID_Implementation(FName NewEquipmentID) override;
	virtual void SetBoneID_Implementation(FName NewBoneID) override;
	virtual void InitializeEquipment_Implementation(FName NewEquipmentID, FName NewBoneID) override;
	virtual  USpringArmComponent* GetSpringArmComponent_Implementation() const override;
	virtual void SetSpringArmComponent_Implementation(USpringArmComponent* SpringArmComponent) override;
	virtual ETGEquipmentCategory GetCategory_Implementation() override;

protected:
	virtual void BeginPlay() override;
	int32 Defense;
	virtual int32 GetDefensePoint_Implementation() override;
	virtual void SetDefensePoint_Implementation(int32 NewDefensePoint) override;
};