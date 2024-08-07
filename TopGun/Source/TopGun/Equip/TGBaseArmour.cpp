#include "TGBaseArmour.h"

#include "Data/EquipmentData.h"

ATGBaseArmour::ATGBaseArmour()
{
	Category = ETGEquipmentCategory::Armour;
	PrimaryActorTick.bCanEverTick = true;
	Defense = 0;
}

void ATGBaseArmour::BeginPlay()
{
	Super::BeginPlay();
}


int32 ATGBaseArmour::GetDefensePoint_Implementation()
{
	return Defense;
}

void ATGBaseArmour::SetDefensePoint_Implementation(int32 NewDefensePoint)
{
	Defense = NewDefensePoint;
}



FName ATGBaseArmour::GetEquipmentID_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("GetEquipmentID_Implementation : EquipID: %s"), *EquipmentID.ToString());
	return EquipmentID;
}

FName ATGBaseArmour::GetBoneID_Implementation()
{
	return BoneID;
}

void ATGBaseArmour::SetEquipmentID_Implementation(FName NewEquipmentID)
{
	UE_LOG(LogTemp, Warning, TEXT("SetEquipmentID_Implementation : EquipID: %s"), *NewEquipmentID.ToString());
	EquipmentID = NewEquipmentID;
}

void ATGBaseArmour::SetBoneID_Implementation(FName NewBoneID)
{
	BoneID = NewBoneID;
}

void ATGBaseArmour::InitializeEquipment_Implementation(FName NewEquipmentID, FName NewBoneID)
{
	EquipmentID = NewEquipmentID;
	BoneID = NewBoneID;
}

USpringArmComponent* ATGBaseArmour::GetSpringArmComponent_Implementation() const
{
	return MySpringArmComponent;
}

void ATGBaseArmour::SetSpringArmComponent_Implementation(USpringArmComponent* SpringArmComponent)
{
	MySpringArmComponent = SpringArmComponent;
}

ETGEquipmentCategory ATGBaseArmour::GetCategory_Implementation()
{
	return Category;
}



