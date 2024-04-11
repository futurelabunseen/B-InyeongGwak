// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MeshMergeFunctionLibrary.h"
#include "ABCharacterBase.generated.h"

UENUM()
enum class ECharacterControlType : uint8
{
	Shoulder,
	Quater
};

UENUM(BlueprintType)
enum class E_PartsCode : uint8
{
	Head UMETA(DisplayName = "Head"),
	UpperBody UMETA(DisplayName = "UpperBody"),
	LowerBody UMETA(DisplayName = "LowerBody"),
	Hands UMETA(DisplayName = "Hands")
	
};


UCLASS()
class ARENABATTLE_API AABCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AABCharacterBase();
	UFUNCTION(BlueprintCallable, Category="Character Customization")
	void IncrementAndSelectPart(E_PartsCode PartCode);
	UFUNCTION(BlueprintCallable, Category="Character Customization")
	void MergeCharacterParts();


protected:
	virtual void SetCharacterControlData(const class UABCharacterControlData* CharacterControlData);

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Customization")
	TMap<ECharacterControlType, class UABCharacterControlData*> CharacterControlManager;
//ComboAction
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	TObjectPtr<class UAnimMontage> ComboActionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Attack, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UABComboActionData> ComboActionData;

	void ProcessComboCommand();

	void ComboActionBegin();
	void ComboActionEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);
	void SetComboCheckTimer();
	void ComboCheck();
	
	int32 CurrentCombo = 0;
	FTimerHandle ComboTimerHandle;
	bool HasNextComboCommand = false;

	//BodyParts
	FString HeadBodyDir = TEXT("/Game/SCK_Casual01/Models/Head");
	FString UpperBodyDir = TEXT("/Game/SCK_Casual01/Models/UpperBody");
	FString LowerBodyDir = TEXT("/Game/SCK_Casual01/Models/LowerBody");
	FString HandsDir = TEXT("/Game/SCK_Casual01/Models/Hands");
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Parts")
	TMap<E_PartsCode, USkeletalMeshComponent*> CharacterPartsMap;
	TSubclassOf<UAnimInstance> MyAnimInstanceClass;
	TMap<E_PartsCode, TArray<TSoftObjectPtr<USkeletalMesh>>> AvailableMeshesForParts;
	void InitializeAvailableBodyParts();
	void LoadMeshesFromDirectory(const FString& DirectoryPath, E_PartsCode);
	USkeletalMesh* SetUpPartsForMerge(E_PartsCode);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= BodyParts)
	TMap<E_PartsCode, int32> BodyPartIndex;

	void SelectRandomPart(E_PartsCode PartCode);
	

	void SelectPartByIndex(E_PartsCode PartCode, int32 Index);
};

