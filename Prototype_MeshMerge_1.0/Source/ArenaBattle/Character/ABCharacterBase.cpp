#include "Character/ABCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABCharacterControlData.h"
#include "Animation/AnimMontage.h"
#include "ABComboActionData.h"
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

void AABCharacterBase::ProcessComboCommand()
{
	if (CurrentCombo == 0)
	{
		ComboActionBegin();
		return;
	}

	if(!ComboTimerHandle.IsValid())
	{
		HasNextComboCommand = false;
	} else
	{
		HasNextComboCommand = true;
	}
}

void AABCharacterBase::ComboActionBegin()
{
	CurrentCombo = 1;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	const float AttackSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh() -> GetAnimInstance();
	AnimInstance->Montage_Play(ComboActionMontage, AttackSpeedRate);


	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AABCharacterBase::ComboActionEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboActionMontage);

	ComboTimerHandle.Invalidate();
	SetComboCheckTimer();
}

void AABCharacterBase::ComboActionEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	ensure(CurrentCombo!=0);
	CurrentCombo = 0;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

}

void AABCharacterBase::SetComboCheckTimer()
{
	int32 ComboIndex = CurrentCombo - 1;
	ensure(ComboActionData->EffectiveFrameCount.IsValidIndex(ComboIndex));

	const float AttackSpeedRate = 1.0f;
	float ComboEffectiveTime = (ComboActionData->EffectiveFrameCount[ComboIndex] / ComboActionData->FrameRate) / AttackSpeedRate;
	if (ComboEffectiveTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &AABCharacterBase::ComboCheck, ComboEffectiveTime, false);
	}
}

void AABCharacterBase::ComboCheck()
{
	ComboTimerHandle.Invalidate();
	if(HasNextComboCommand)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		CurrentCombo = FMath::Clamp(CurrentCombo + 1, 1, ComboActionData->MaxComboCount);
		FName NextSection = *FString::Printf(TEXT("%s%d"), *ComboActionData->MontageSectionNamePrefix, CurrentCombo);
		AnimInstance->Montage_JumpToSection(NextSection,ComboActionMontage);
		SetComboCheckTimer();
		HasNextComboCommand = false;
	}
}

//Module Character Parts Code
void AABCharacterBase::InitializeAvailableBodyParts()
{
	LoadMeshesFromDirectory(HeadBodyDir, TEXT("Head"));
	LoadMeshesFromDirectory(UpperBodyDir, TEXT("UpperBody"));
	LoadMeshesFromDirectory(LowerBodyDir, TEXT("LowerBody"));
	LoadMeshesFromDirectory(HandsDir, TEXT("Hands"));

}

void AABCharacterBase::LoadMeshesFromDirectory(const FString& DirectoryPath, const FString& PartType)
{
	TArray<TSoftObjectPtr<USkeletalMesh>> Meshes;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName()); // Updated line
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
	FSkeletalMeshMerge MeshMerger(MergedMesh, SelectingPartsForMerge(), SectionMappings, StripTopLODs);
	if (MeshMerger.DoMerge())
	{
		GetMesh()->SetSkeletalMesh(MergedMesh);
		GetMesh()->SetAnimInstanceClass(MyAnimInstanceClass);
	}
}

TArray<USkeletalMesh*> AABCharacterBase::SelectingPartsForMerge()
{
	TArray<USkeletalMesh*> PartsToMerge;

	for (const auto& Elem : AvailableMeshesForParts)
	{
		const FString& PartName = Elem.Key;
		const TArray<TSoftObjectPtr<USkeletalMesh>>& Variants = Elem.Value;

		if (Variants.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, Variants.Num() - 1);
			USkeletalMesh* SelectedMesh = Variants[RandomIndex].LoadSynchronous();
			if (SelectedMesh)
			{
				PartsToMerge.Add(SelectedMesh);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No variants available for part '%s'"), *PartName);
		}
	}

	return PartsToMerge;
}









