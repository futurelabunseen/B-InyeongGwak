// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/TGModuleDataAsset.h"

USkeletalMesh* UTGModuleDataAsset::GetMeshByID(FName PartCode) const
{
	const FMeshCategoryData* MeshData = BaseMeshComponent.Find(PartCode);
	if (MeshData)
	{
		return MeshData->TargetSkeletalMesh;
	}
	return nullptr;
}