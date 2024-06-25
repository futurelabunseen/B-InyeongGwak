// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Character/TGCharacterBase.h"
#include "TopGun/Utility/TGModulesystem.h"
#include "GameInstance/TGGameInstance.h"
#include "Weapon/TGBaseWeapon.h"

#include "TGCharacterPlayer.generated.h"
/**
 * 
 */

UENUM()
enum class ECharacterControlType : uint8
{
	Flying,
	Walking,
	Changing
};


UCLASS()
class TOPGUN_API ATGCharacterPlayer : public ATGCharacterBase
{
	GENERATED_BODY()
public :
	ATGCharacterPlayer();
	virtual ~ATGCharacterPlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> PlayerMesh;

protected:
	virtual void BeginPlay() override;
	void SetupPlayerModel(USkeletalMeshComponent* TargetMesh);
	void AttackCall(bool isFiring);
	void Walk(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ResetWeaponRotations();
	void SetWeaponRotations();

	virtual void Tick(float DeltaSeconds) override;
	
	//CharacterSetting
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> ShoulderCameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	// Character Control Section
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void SwitchScene();
	void AttackStart();
	void AttackEnd();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraOffsetWhenAiming = FVector(50.0f, 50.0f, 0.0f);;
	
protected:
	ECharacterControlType CurrentCharacterControlType;
	void ChangeCharacterControl();
	void AimCameraStart();
	FVector GetScreenAimingPointVector() const;
	void AimCameraEnd();
	void SetCharacterControl(ECharacterControlType NewCharacterControlType);
	virtual void SetCharacterControlData(const class UTGPlayerControlData* CharacterControlData) override;
	virtual void Die() override;
	
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
	TObjectPtr<class UInputAction> SwitchSceneAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AimAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = State, Meta = (AllowPrivateAccess = "true"))
	bool bIsAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Control")
	TMap<ECharacterControlType, class UTGPlayerControlData*> CharacterControlManager;
	
	void FlyingMove(const FInputActionValue& Value);
	void FlyingLook(const FInputActionValue& Value);

private:
	TMap<E_PartsCode, int32> BodyPartIndex;
	TWeakObjectPtr<UTGCGameInstance> MyGameInstance;
	TMap<FName, AActor*> WeaponMap;
	UFUNCTION()
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	UFUNCTION()
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	
public:
	UPROPERTY()
	TMap<FString, AActor*> SocketActors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterSocket")
	TArray<FString> socketNames = { TEXT("HeadSocket"), TEXT("HandSocket"), TEXT("FootSocket") };
private:
	FVector OriginalCameraOffset;
	FVector TargetCameraOffset;
	float InterpSpeed = 5.0f;	
};
