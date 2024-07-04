// TGFlyingCharacterPlayer.h
#pragma once

#include "CoreMinimal.h"
#include "Character/TGCharacterPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TGFlyingCharacterPlayer.generated.h"

UCLASS()
class TOPGUN_API ATGFlyingCharacterPlayer : public ATGCharacterBase
{
    GENERATED_BODY()

public:
    ATGFlyingCharacterPlayer();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;
    void SetCharacterControl() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UInputMappingContext> DefaultMappingContext;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USpringArmComponent> ShoulderCameraBoom;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerMesh, Meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> PlayerMesh;

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float DesiredFOV;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector DesiredSocketOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float FlightRotationInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    bool bBoost;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Flight")
    FRotator FlyingRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flight")
    FRotator LastVelocityRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    bool bIsFlying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    FVector DefaultSocketOffset;

    UFUNCTION(BlueprintCallable, Category = "Flying")
    void EnableFlight();

    UFUNCTION(BlueprintCallable, Category = "Flying")
    void DisableFlight();

   
    
    // Input Actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* BoostAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    class UInputAction* SwitchSceneAction;
    

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Move(const FInputActionValue& Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    virtual void Look(const FInputActionValue& Value);

    virtual void Jump() override;

    virtual void StopJumping() override;

    void Boost(const FInputActionValue& Value);

    virtual void SwitchScene();

private:
    void UpdateCameraAndArm(float DeltaTime);
    void UpdateRotation(float DeltaTime);
    void SmoothRotation(const FRotator& Target, float ConstantSpeed, float SmoothSpeed, float DeltaTime);
    FRotator CalculateTargetRotation() const;
    bool ShouldUseLastVelocityRotation() const;
};
