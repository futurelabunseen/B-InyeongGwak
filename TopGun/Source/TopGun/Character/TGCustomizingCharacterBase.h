#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Interface/TGCharacterWidgetInterface.h"
#include "Utility/TGModuleSystem.h"
#include "TGCustomizingCharacterBase.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOPGUN_API ATGCustomizingCharacterBase : public ACharacter, public ITGCharacterWidgetInterface
{
	GENERATED_BODY()
public:
	ATGCustomizingCharacterBase();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* mySkeletalMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	class UTGCustomizingComponent* CustomizingComponent;

	UPROPERTY()
	TWeakObjectPtr<UTGCGameInstance> MyGameInstance;

	UPROPERTY()
	TMap<E_PartsCode, USkeletalMeshComponent*> CharacterPartsMap;

	UFUNCTION(BlueprintCallable)
	void ResetGameCustomization();

	// In TGCustomizingCharacterBase.h
	private:
	template<typename T>
	void SetupActors(USkeletalMeshComponent* TargetMesh, const TMap<FName, FAttachedActorData>& ActorMap, const TMap<FName, UBlueprintGeneratedClass*>& ClassMap, const FString& ActorType) const;
	
protected:
	UFUNCTION(BlueprintCallable)
	virtual void SetupCharacterWidget(UTGUserWidget* InUserWidget) override;

	virtual void BeginPlay() override;
	void SetupPlayerModel(USkeletalMeshComponent* TargetMesh) const;
	void SetupCharacterParts() const;
	void SetupWeapons(USkeletalMeshComponent* TargetMesh) const;
	bool SetupSingleWeapon(UWorld* World, USkeletalMeshComponent* TargetMesh, const FName& WeaponID,
	                       const FName& BoneID,
	                       const FRotator& Rotation) const;
	void SetupArmour(USkeletalMeshComponent* TargetMesh) const;
	virtual void Tick(float DeltaSeconds) override;
};
