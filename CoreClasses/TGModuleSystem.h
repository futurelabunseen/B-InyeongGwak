#pragma once

#include "CoreMinimal.h"
#include "TGModuleSystem.generated.h"

class UTGModuleDataAsset;

UENUM(BlueprintType)
enum class E_PartsCode : uint8
{
	Head UMETA(DisplayName = "Head"),
	UpperBody UMETA(DisplayName = "Upper Body"),
	LowerBody UMETA(DisplayName = "Lower Body"),
	LeftHand UMETA(DisplayName = "Left Hand"),
	RightHand UMETA(DisplayName = "Right Hand")
};

UCLASS(BlueprintType)
class TOPGUN_API UTGModuleSystem : public UObject
{
	GENERATED_BODY()

public:
	UTGModuleSystem();

	UFUNCTION(BlueprintCallable, Category = "Character Customization")
	static USkeletalMesh* GetMergeCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TSoftObjectPtr<UTGModuleDataAsset> ModuleAsset);

	void SetupModuleFromLeaderPoseComponent(const TMap<E_PartsCode, FName>& WholeModuleData, TSoftObjectPtr<UTGModuleDataAsset> ModuleAsset);
};
