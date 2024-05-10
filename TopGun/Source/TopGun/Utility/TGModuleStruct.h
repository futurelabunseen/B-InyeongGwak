
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TGModuleStruct.generated.h"

USTRUCT(BlueprintType)
struct FTGModuleStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterModule")
	USkeletalMeshComponent *TargetMesh;

};