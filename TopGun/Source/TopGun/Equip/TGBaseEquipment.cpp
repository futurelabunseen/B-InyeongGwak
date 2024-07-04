// TGBaseEquipment.cpp
#include "TGBaseEquipment.h"

ATGBaseEquipment::ATGBaseEquipment()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATGBaseEquipment::BeginPlay()
{
	Super::BeginPlay();
}

void ATGBaseEquipment::InitializeEquipment(FName NewEquipmentID, FName NewBoneID)
{
	EquipmentID = NewEquipmentID;
	BoneID = NewBoneID;
}

void ATGBaseEquipment::SetSpringArmComponent(USpringArmComponent* SpringArmComponent)
{
	MySpringArmComponent = SpringArmComponent;
}

USpringArmComponent* ATGBaseEquipment::GetSpringArmComponent() const
{
	return MySpringArmComponent;
}

void ATGBaseEquipment::SetEquipmentID(FName NewEquipmentID)
{
	EquipmentID = NewEquipmentID;
}

void ATGBaseEquipment::SetBoneID(FName NewBoneID)
{
	BoneID = NewBoneID;
}

FName ATGBaseEquipment::GetEquipmentID() const
{
	return EquipmentID;
}

FName ATGBaseEquipment::GetBoneID() const
{
	return BoneID;
}