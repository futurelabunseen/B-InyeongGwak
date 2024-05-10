// TGModuleSystem.h

#pragma once

#include "CoreMinimal.h"
#include "TGModuleSystem.generated.h"

class UTGModuleDataAsset;

UENUM(BlueprintType)
enum class E_PartsCode : uint8
{
	Head UMETA(DisplayName = "Head"),
	UpperBody UMETA(DisplayName = "Chest"),
	LowerBody UMETA(DisplayName = "Leg"),
	LeftHand UMETA(DisplayName = "LeftArm"),  
	RightHand UMETA(DisplayName = "RightArm")
};

UCLASS(BlueprintType)
class TOPGUN_API UTGModuleSystem : public UObject
{
	GENERATED_BODY()

public:
	UTGModuleSystem();
	UFUNCTION(BlueprintCallable, Category="Character Customization")
	USkeletalMesh* GetMergeCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TSoftObjectPtr<UTGModuleDataAsset> ModuleAsset);
};
