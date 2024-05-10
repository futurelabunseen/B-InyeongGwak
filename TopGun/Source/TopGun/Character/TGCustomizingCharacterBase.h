#pragma once

#include "CoreMinimal.h"
#include "Components/ScrollBox.h"
#include "GameFramework/Character.h"
#include "Utility/TGModuleSystem.h"
#include "Weapon/TGBaseWeapon.h"
#include "TGCustomizingCharacterBase.generated.h"

UENUM(BlueprintType)
enum class ECustomizingState : uint8
{
	Idle,
	OnDragActor,
	OnSnappedActor
};


UCLASS()
class TOPGUN_API ATGCustomizingCharacterBase : public ACharacter
{
	GENERATED_BODY()
	//==VARIABLES========================================================
public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	//WeaponDataAsset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDataAsset> WeaponDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDataAsset> ModuleDataAsset;
	TSubclassOf<ATGBaseWeapon> CurrentWeaponClass;

	//UI Related
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> ButtonWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponData, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> ModuleButtonWidgetClass;

protected:
	//Referance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* mySkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	APlayerController* myPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponAttach")
    TWeakObjectPtr<AActor> CurrentSpawnedActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponAttach")
	FName CurrentTargetBone;

	const float SnapCheckDistance = 10.0f;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
	ECustomizingState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> LClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> RClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> EscapeAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> RotateAction;

	//==FUNCTIONS=======================================================
private:
	ATGCustomizingCharacterBase();

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	
private:
	//Weapon Attach
	void UpdateWeaponActorPosition() const;
	void RemoveWeaponInDesiredPosition();
	static void RemoveWeaponFromCharacter(ATGBaseWeapon* WeaponToRemove);
	void AttachWeapon();
	void CheckWeaponActorProximity();
	void CheckSnappedCancellation();

	//Input Related
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void OnClickRightMouse();
	void OnClickLeftMouse();
	void OnClickEscape();
	void OnRotateAction(const FInputActionValue& Value);

	//State Management
	void ReturnToIdleState();

public:
	//UI
	UFUNCTION(BlueprintCallable)
	void AddButtonToPanel(UScrollBox* TargetPanel);
	UFUNCTION(BlueprintCallable)
	void AddModuleButtonToPanel(UScrollBox* TargetPanel);
	
	UFUNCTION(BlueprintCallable)
	void OnWeaponSelected(FName WeaponID);
	UFUNCTION(BlueprintCallable)
	void OnModuleSelected(FName WeaponID);
public:
	//BodyParts
	//DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= BodyParts)
	TMap<E_PartsCode, FName> BodyPartIndex;

	static USkeletalMesh* GetMergeCharacterParts(const TMap<E_PartsCode, FName>& WholeModuleData, TSoftObjectPtr<UTGModuleDataAsset> ModuleAsset);
	UClass* MyUAnimClass;
	
};
