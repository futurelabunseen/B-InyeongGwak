// TGBaseEquipment.h
#pragma once

#include "CoreMinimal.h"
#include "Data/EquipmentData.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "TGBaseEquipment.generated.h"

UCLASS(Abstract)
class TOPGUN_API ATGBaseEquipment : public AActor
{
	GENERATED_BODY()
public:
	ATGBaseEquipment();

protected:
	USpringArmComponent* MySpringArmComponent;
	UPROPERTY()
	FName EquipmentID;
	UPROPERTY()
	FName BoneID;
	UPROPERTY()
	ETGEquipmentCategory Category;
};