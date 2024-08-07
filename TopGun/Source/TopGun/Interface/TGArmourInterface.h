// TGArmourInterface.h
#pragma once

#include "CoreMinimal.h"
#include "TGBaseEquipmentInterface.h"
#include "TGArmourInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTGArmourInterface : public UTGBaseEquipmentInterface
{
	GENERATED_BODY()
};

class TOPGUN_API ITGArmourInterface : public ITGBaseEquipmentInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	int32 GetDefensePoint();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetDefensePoint(int32 NewDefensePoint);
};