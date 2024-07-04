// TGBaseEquipment.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "TGBaseEquipment.generated.h"

UCLASS(Abstract)
class TOPGUN_API ATGBaseEquipment : public AActor
{
	GENERATED_BODY()

public:
	ATGBaseEquipment();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FName EquipmentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FName BoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	USpringArmComponent* MySpringArmComponent;

	virtual void InitializeEquipment(FName NewEquipmentID, FName NewBoneID);
	virtual void SetSpringArmComponent(USpringArmComponent* SpringArmComponent);
	virtual USpringArmComponent* GetSpringArmComponent() const;

	virtual void SetEquipmentID(FName NewEquipmentID);
	virtual void SetBoneID(FName NewBoneID);
	virtual FName GetEquipmentID() const;
	virtual FName GetBoneID() const;

protected:
	virtual void BeginPlay() override;
};