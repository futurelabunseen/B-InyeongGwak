#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/TGArmourInterface.h"
#include "TGBaseArmour.generated.h"

UCLASS()
class TOPGUN_API ATGBaseArmour : public AActor, public ITGArmourInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATGBaseArmour();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	int32 Defense = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armour")
	FName ArmourID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armour")
	FName BoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armour")
	USpringArmComponent* MySpringArmComponent;

	virtual void InitializeArmour(FName NewArmourID, FName NewBoneID) override;

	virtual void SetSpringArmComponent(USpringArmComponent* SpringArmComponent) override;
	virtual USpringArmComponent* GetSpringArmComponent() const override;

	virtual void SetArmourID_Implementation(FName NewArmourID) override;
	virtual void SetBoneID_Implementation(FName NewBoneID) override;
	virtual FName GetArmourID_Implementation() override;
	virtual FName GetBoneID_Implementation() override;

protected:
	virtual void BeginPlay() override;
};
