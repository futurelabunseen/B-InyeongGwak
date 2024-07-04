#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/SpringArmComponent.h"
#include "TGArmourInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTGArmourInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOPGUN_API ITGArmourInterface
{
	GENERATED_BODY()

	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetArmourID(FName WeaponID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetBoneID(FName BoneID);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FName GetArmourID();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FName GetBoneID();
	
	virtual void InitializeArmour(FName ArmourID, FName BoneID) = 0;

	virtual void SetSpringArmComponent(USpringArmComponent* SpringArmComponent) = 0;

	virtual USpringArmComponent* GetSpringArmComponent() const = 0;
};
