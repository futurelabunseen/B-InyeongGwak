// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TGCharacterBase.generated.h"

UENUM()
enum class ECharacterControlType : uint8
{
	Flying,
	Walking,
	Changing
};

UCLASS()
class TOPGUN_API ATGCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ATGCharacterBase();

protected:
	virtual void SetCharacterControlData(const class UTGPlayerControlData* CharacterControlData);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Control")
	TMap<ECharacterControlType, class UTGPlayerControlData*> CharacterControlManager;
};
