// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Character/TGCharacterBase.h"
#include "Game/TGGameMode.h"
#include "TopGun/Utility/TGModulesystem.h"
#include "GameInstance/TGGameInstance.h"
#include "Player/TGCustomizingPlayerController.h"
#include "Utility/TGEquipmentManager.h"
#include "Utility/TGFlyingComponent.h"

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
	bool bIsTransitioning;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flying")
	UTGFlyingComponent* FlyingComponent;

private :
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump();
	void Boost(const FInputActionValue& Value);
	
protected:
	virtual void BeginPlay() override;
	void SetCharacterControl() const;
	void SetupPlayerModel(USkeletalMeshComponent* TargetMesh);
	void SetupMesh(USkeletalMeshComponent* TargetMesh);
	void AttachIndividualActor(USkeletalMeshComponent* TargetMesh, FEquipmentKey TargetKey, FAttachedActorData TargetInfo);
	void AttachEquip(USkeletalMeshComponent* TargetMesh);
	
	void AttackCall(bool isFiring);
	void FireWeapon(AActor* WeaponActor, bool isFiring);
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
	void ProcessWeaponSelectionWrapper(const FInputActionValue& Value, FKey PressedKey);

	UFUNCTION(BlueprintCallable)
	void SwitchScene();
	void PrepareForLevelTransition();

	void AttackStart();
	void AttackEnd();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraOffsetWhenAiming = FVector(50.0f, 50.0f, 0.0f);;

	UFUNCTION(BlueprintCallable)
	void SetUpGameOverWidget(UTGUserWidget* InUserWidget);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWidget")
	TSubclassOf<UUserWidget> PlayerHUDTemplate;
	
	//Key Binding
	UFUNCTION()
	void ProcessWeaponSelection(FKey PressedKey);
	
protected:
	void ChangeCharacterControl();
	void AimCameraStart();
	FVector GetScreenAimingPointVector() const;
	void AimCameraEnd();
	virtual void SetCharacterControlData(const class UTGPlayerControlData* CharacterControlData) override;
	virtual void Die() override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	//HUD
	virtual void SetupCharacterWidget(UTGUserWidget* InUserWidget) override;
	bool bIsInvincible;
	FTimerHandle InvincibilityTimerHandle;

	void ResetInvincibility();

	// Input Section
	
	protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> FlyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* BoostAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> SwitchSceneAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<class UInputAction>> NumPadInputActions;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = State, Meta = (AllowPrivateAccess = "true"))
	bool bIsAiming = false;

	
	
private:
	FKey CurrentSelectedWeapon = EKeys::Zero;
	
private:
	TMap<E_PartsCode, int32> BodyPartIndex;
	TWeakObjectPtr<UTGCGameInstance> MyGameInstance;
	TWeakObjectPtr<ATGGameMode> MyGameMode;
	
	TMap<AActor*, FName> WeaponMap;
	TMap<AActor*, FName> ArmourMap;
	UFUNCTION()
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	UFUNCTION()
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	bool isDead = false;
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