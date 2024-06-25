#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Utility/TGModuleSystem.h"
#include "TGCustomizingCharacterBase.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOPGUN_API ATGCustomizingCharacterBase : public ACharacter
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
	
protected:
	virtual void BeginPlay() override;
	void SetupPlayerModel(USkeletalMeshComponent* TargetMesh) const;
	virtual void Tick(float DeltaSeconds) override;

	
};
