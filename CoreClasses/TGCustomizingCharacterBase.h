#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interface/TGCharacterWidgetInterface.h"
#include "Utility/TGModuleSystem.h"
#include "Stats/Stats.h"
#include "GameInstance/TGGameInstance.h"
#include "TGCustomizingCharacterBase.generated.h"

DECLARE_STATS_GROUP(TEXT("CharacterCustomizing"), STATGROUP_Sample, STATCAT_Advanced);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOPGUN_API ATGCustomizingCharacterBase : public ACharacter, public ITGCharacterWidgetInterface
{
    GENERATED_BODY()
    
public:
    ATGCustomizingCharacterBase();
    
    // Camera Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class USpringArmComponent* CameraBoom;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class UCameraComponent* FollowCamera;
    
    // Mesh & Customization Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* mySkeletalMeshComponent;
    
    UPROPERTY()
    TWeakObjectPtr<UTGCGameInstance> MyGameInstance;
    
    UPROPERTY()
    TMap<E_PartsCode, USkeletalMeshComponent*> CharacterPartsMap;
    
    UFUNCTION(BlueprintCallable)
    void ResetGameCustomization();

protected:
    UFUNCTION(BlueprintCallable)
    virtual void SetupCharacterWidget(UTGUserWidget* InUserWidget) override;
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    
    // Setup functions
    void SetupEquip(USkeletalMeshComponent* TargetMesh) const;
    bool SpawnEquip(USkeletalMeshComponent* TargetMesh, const FName& WeaponID, const FName& BoneID, const FRotator& Rotation) const;
    void SetupPlayerModel(USkeletalMeshComponent* TargetMesh) const;
    void SetupCharacterParts() const;
};
