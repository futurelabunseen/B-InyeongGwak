#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Utility/TGModuleSystem.h"
#include "TGCustomizingCharacterBase.generated.h"

UCLASS()
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
	class UTGCustomizingComponent* CustomizingComponent;
	TWeakObjectPtr<UTGCGameInstance> MyGameInstance;

protected:
	virtual void BeginPlay() override;
	void SetupPlayerModel(USkeletalMeshComponent* TargetMesh) const;
	virtual void Tick(float DeltaSeconds) override;
};
