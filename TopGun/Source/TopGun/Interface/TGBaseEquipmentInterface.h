// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/SpringArmComponent.h"
#include "TGBaseEquipmentInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTGBaseEquipmentInterface : public UInterface
{
    GENERATED_BODY()
};

class TOPGUN_API ITGBaseEquipmentInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
    FName GetEquipmentID();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
    FName GetBoneID();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
    ETGEquipmentCategory GetCategory();
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
    void SetEquipmentID(FName EquipmentID);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
    void SetBoneID(FName BoneID);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
    void InitializeEquipment(FName EquipmentID, FName BoneID);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
    USpringArmComponent* GetSpringArmComponent() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Equipment")
    void SetSpringArmComponent(USpringArmComponent* SpringArmComponent);

};