// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/TGModuleSystem.h"
#include "SkeletalMeshMerge.h"
#include "TGModuleDataAsset.h"


UTGModuleSystem::UTGModuleSystem()
{
	
}

USkeletalMesh* UTGModuleSystem::GetMergeCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TSoftObjectPtr<UTGModuleDataAsset> ModuleAsset)
{
	TArray<USkeletalMesh*> PartsToMerge;
	for (const auto& Elem : WholeModuleData)
	{
		PartsToMerge.Add(ModuleAsset->GetMeshByID(Elem.Value));
	}
	if (PartsToMerge.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No parts to merge, operation cancelled."));
		return nullptr;
	}
	USkeletalMesh* MergedMesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!MergedMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create a merged skeletal mesh."));
		return nullptr;
	}
	FSkeletalMeshMerge MeshMerger(MergedMesh, PartsToMerge, TArray<FSkelMeshMergeSectionMapping>(), 0);
	bool bMergeSuccess = MeshMerger.DoMerge();
	if (bMergeSuccess)
	{
		return MergedMesh;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh merge failed."));
		return nullptr;
	}
}
