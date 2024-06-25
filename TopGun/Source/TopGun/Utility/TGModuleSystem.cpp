// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/TGModuleSystem.h"
#include "SkeletalMeshMerge.h"
#include "TGModuleDataAsset.h"
#include "Character/TGCustomizingCharacterBase.h"


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
		UE_LOG(LogTemp, Log, TEXT("PartsToMerge  Empty"));
		return nullptr;
	}
	USkeletalMesh* MergedMesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!MergedMesh)
	{
		UE_LOG(LogTemp, Log, TEXT("MergedMesh  Empty"));
		return nullptr;
	}
	FSkeletalMeshMerge MeshMerger(MergedMesh, PartsToMerge, TArray<FSkelMeshMergeSectionMapping>(), 0);
	if (bool bMergeSuccess = MeshMerger.DoMerge())
	{
		return MergedMesh;
	} else
	{
		UE_LOG(LogTemp, Log, TEXT("Merge Mesh Failed"));
		return nullptr;
	}
}

