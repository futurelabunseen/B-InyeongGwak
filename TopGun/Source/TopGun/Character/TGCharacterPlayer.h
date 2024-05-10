// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Character/TGCharacterBase.h"
#include "TopGun/Utility/TGModulesystem.h"
#include "TGCharacterPlayer.generated.h"
/**
 * 
 */
UCLASS()
class TOPGUN_API ATGCharacterPlayer : public ATGCharacterBase
{
	GENERATED_BODY()
public :
	ATGCharacterPlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> PlayerMesh;

protected:
	virtual void BeginPlay() override;
	void Walk(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Fly(const FInputActionValue& Value);


	
	//CharacterSetting
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	// Character Control Section
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
protected:
	ECharacterControlType CurrentCharacterControlType;
	void ChangeCharacterControl();
	void SetCharacterControl(ECharacterControlType NewCharacterControlType);
	virtual void SetCharacterControlData(const class UTGPlayerControlData* CharacterControlData) override;
	
	// Input Section
	
	protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ChangeControlAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> FlyAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> EnterEditAction;

	void WalkingMove(const FInputActionValue& Value);
	void WalkingLook(const FInputActionValue& Value);
	void FlyingMove(const FInputActionValue& Value);
	void FlyingLook(const FInputActionValue& Value);

private:
	TMap<E_PartsCode, int32> BodyPartIndex;

public:
	UPROPERTY()
	TMap<FString, AActor*> SocketActors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterSocket")
	TArray<FString> socketNames = { TEXT("HeadSocket"), TEXT("HandSocket"), TEXT("FootSocket") };
};
