#include "TGBaseArmour.h"

ATGBaseArmour::ATGBaseArmour()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATGBaseArmour::BeginPlay()
{
	Super::BeginPlay();
}

void ATGBaseArmour::InitializeArmour(FName NewArmourID, FName NewBoneID)
{
	this->ArmourID = NewArmourID;
	this->BoneID = NewBoneID;
}

void ATGBaseArmour::SetSpringArmComponent(USpringArmComponent* SpringArmComponent)
{
	MySpringArmComponent = SpringArmComponent;
}

USpringArmComponent* ATGBaseArmour::GetSpringArmComponent() const
{
	return MySpringArmComponent;
}

void ATGBaseArmour::SetArmourID_Implementation(FName NewArmourID)
{
	this->ArmourID = NewArmourID;
}

void ATGBaseArmour::SetBoneID_Implementation(FName NewBoneID)
{
	this->BoneID = NewBoneID;
}

FName ATGBaseArmour::GetArmourID_Implementation()
{
	return ArmourID;
}

FName ATGBaseArmour::GetBoneID_Implementation()
{
	return BoneID;
}
