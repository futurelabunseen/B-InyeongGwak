#include "Character/ABCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABCharacterControlData.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "MeshMergeFunctionLibrary.h"
#include "SkeletalMeshMerge.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"

// Sets default values
AABCharacterBase::AABCharacterBase()
{
	// Pawn
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

	
	
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(TEXT("/Game/ArenaBattle/Animation/ABP_ABCharacter.ABP_ABCharacter_C"));
	if (AnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
		MyAnimInstanceClass = AnimInstanceClassRef.Class;
	}

	static ConstructorHelpers::FObjectFinder<UABCharacterControlData> ShoulderDataRef(TEXT("/Script/ArenaBattle.ABCharacterControlData'/Game/ArenaBattle/CharacterControl/ABC_Shoulder.ABC_Shoulder'"));
	if (ShoulderDataRef.Object)
	{
		CharacterControlManager.Add(ECharacterControlType::Shoulder, ShoulderDataRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UABCharacterControlData> QuaterDataRef(TEXT("/Script/ArenaBattle.ABCharacterControlData'/Game/ArenaBattle/CharacterControl/ABC_Quater.ABC_Quater'"));
	if (QuaterDataRef.Object)
	{
		CharacterControlManager.Add(ECharacterControlType::Quater, QuaterDataRef.Object);
	}
	InitializeAvailableBodyParts();
	
}

void AABCharacterBase::SetCharacterControlData(const UABCharacterControlData* CharacterControlData)
{
	// Pawn
	bUseControllerRotationYaw = CharacterControlData->bUseControllerRotationYaw;
	// CharacterMovement
	GetCharacterMovement()->bOrientRotationToMovement = CharacterControlData->bOrientRotationToMovement;
	GetCharacterMovement()->bUseControllerDesiredRotation = CharacterControlData->bUseControllerDesiredRotation;
	GetCharacterMovement()->RotationRate = CharacterControlData->RotationRate;
}

void AABCharacterBase::InitializeAvailableBodyParts()
{
	LoadMeshesFromDirectory(HeadBodyDir, E_PartsCode::Head);
	LoadMeshesFromDirectory(UpperBodyDir, E_PartsCode::UpperBody);
	LoadMeshesFromDirectory(LowerBodyDir, E_PartsCode::LowerBody);
	LoadMeshesFromDirectory(HandsDir, E_PartsCode::Hands);

	BodyPartIndex.Add(E_PartsCode::Head, 0);
	BodyPartIndex.Add(E_PartsCode::UpperBody, 0);
	BodyPartIndex.Add(E_PartsCode::LowerBody, 0);
	BodyPartIndex.Add(E_PartsCode::Hands, 0);
}


void AABCharacterBase::LoadMeshesFromDirectory(const FString& DirectoryPath, E_PartsCode PartType)
{
	TArray<TSoftObjectPtr<USkeletalMesh>> Meshes;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(*DirectoryPath);
	TArray<FAssetData> AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	for (const FAssetData& Asset : AssetList)
	{
		TSoftObjectPtr<USkeletalMesh> targetMesh(Asset.ToSoftObjectPath());
		Meshes.Add(targetMesh);
	}

	AvailableMeshesForParts.Add(PartType, Meshes);
}


void AABCharacterBase::MergeCharacterParts()
{
	UE_LOG(LogTemp, Log, TEXT("Beginning to merge character parts..."));
	TArray<USkeletalMesh*> MeshesToMerge;
	TArray<FSkelMeshMergeSectionMapping> SectionMappings;
	TArray<FSkelMeshMergeMeshUVTransforms> UVTransforms;
	int32 StripTopLODs = 0;
	USkeleton* Skeleton = GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
	USkeletalMesh* MergedMesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);
	MergedMesh->USkeletalMesh::SetSkeleton(Skeleton);
	
	TArray<USkeletalMesh*> PartsToMerge;
	for (const auto& Elem : AvailableMeshesForParts)
	{
		PartsToMerge.Add(SetUpPartsForMerge(E_PartsCode::Head));
		PartsToMerge.Add(SetUpPartsForMerge(E_PartsCode::UpperBody));
		PartsToMerge.Add(SetUpPartsForMerge(E_PartsCode::LowerBody));
		PartsToMerge.Add(SetUpPartsForMerge(E_PartsCode::Hands));
	}
	
	FSkeletalMeshMerge MeshMerger(MergedMesh, PartsToMerge, SectionMappings, StripTopLODs);
	if (MeshMerger.DoMerge())
	{
		GetMesh()->SetSkeletalMesh(MergedMesh);
		GetMesh()->SetAnimInstanceClass(MyAnimInstanceClass);
	} else
	{
		UE_LOG(LogTemp, Log, TEXT("Returned MeshMerge Skeleton NULL"));
	}
}

USkeletalMesh* AABCharacterBase::SetUpPartsForMerge(E_PartsCode PartsCode)
{
	if(!AvailableMeshesForParts.Contains(PartsCode))
	{
		return nullptr;
	}
	const TArray<TSoftObjectPtr<USkeletalMesh>>& Variants = AvailableMeshesForParts[PartsCode];

	if (Variants.Num() > 0)
	{
		USkeletalMesh* SelectedMesh = Variants[BodyPartIndex[PartsCode]].LoadSynchronous();
		return SelectedMesh;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No variants available for part '%s'"), PartsCode);
		return nullptr;
	}
}

void AABCharacterBase::SelectRandomPart(E_PartsCode PartCode)
{
	int32 RandomIndex = FMath::RandRange(0, AvailableMeshesForParts[PartCode].Num() - 1);

	if (AvailableMeshesForParts.Contains(PartCode)) {
		int32 CurrentIndex = BodyPartIndex.Contains(PartCode) ? BodyPartIndex[PartCode] : 0;
		BodyPartIndex[PartCode] = (CurrentIndex + 1) % AvailableMeshesForParts[PartCode].Num();
	}
}

void AABCharacterBase::IncrementAndSelectPart(E_PartsCode PartCode)
{
	if (AvailableMeshesForParts.Contains(PartCode)) {
		int32 CurrentIndex = BodyPartIndex.Contains(PartCode) ? BodyPartIndex[PartCode] : 0;
		BodyPartIndex[PartCode] = (CurrentIndex + 1) % AvailableMeshesForParts[PartCode].Num();
	}
}

void AABCharacterBase::SelectPartByIndex(E_PartsCode PartCode, int32 Index)
{
	if (AvailableMeshesForParts.Contains(PartCode)) {
		BodyPartIndex[PartCode] = Index % AvailableMeshesForParts[PartCode].Num();
	}
}








