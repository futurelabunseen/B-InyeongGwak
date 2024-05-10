// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TGModuleDataAsset.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMeshCategoryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Category")
	USkeletalMesh * TargetSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Category")
	E_PartsCode Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Category")
	int32 stat;
};

UCLASS()
class TOPGUN_API UTGModuleDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Object)
	TMap<FName, FMeshCategoryData> BaseMeshComponent;

	USkeletalMesh* GetMeshByID(FName PartCode) const;
};
