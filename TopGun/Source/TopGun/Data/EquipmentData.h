#pragma once

#include <CoreMinimal.h>
#include <EquipmentData.generated.h>



UENUM()
enum class ETGEquipmentCategory : uint8
{
	Armour,
	Weapon,
	Moudle,
	None
};

USTRUCT(BlueprintType)
struct FEquipmentData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Equipment)
	FName EquipmentName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Equipment)
	ETGEquipmentCategory EquipmentCategory; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Equipment)
	UBlueprintGeneratedClass* BaseEquipmentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Equipment)
	int32 points = 0; 
};
