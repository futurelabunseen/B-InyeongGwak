#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TGFlyingComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TOPGUN_API UTGFlyingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTGFlyingComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void EnableFlight();
    void DisableFlight();
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void ToggleFlight();
    void Boost(const FInputActionValue& Value);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    bool bIsFlying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    bool bBoost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    float DesiredFOV;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    FVector DesiredSocketOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    FVector DefaultSocketOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    float FlightRotationInterpSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Flying")
    FRotator LastVelocityRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    float GroundBrakingDeceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    float FlyingBrakingDeceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    float NormalFlySpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flying")
    float BoostFlySpeed;

protected:
    virtual void BeginPlay() override;

private:
    void UpdateCameraAndArm(float DeltaTime);
    void UpdateRotation(float DeltaTime);
    void SmoothRotation(const FRotator& Target, float ConstantSpeed, float SmoothSpeed, float DeltaTime);
    FRotator CalculateTargetRotation() const;
    bool ShouldUseLastVelocityRotation() const;
    bool IsOnGround() const;
    void UpdateGroundDetection();
    void StartStiffDeceleration();
    void ApplyStiffDeceleration(float DeltaTime);

    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* CharacterMovement;

    UPROPERTY()
    UCameraComponent* FollowCamera;

    UPROPERTY()
    USpringArmComponent* CameraBoom;

    FRotator FlyingRotation;

    float BoostDuration;
    float MaxBoostDuration;
    float StiffDecelerationMultiplier;
    bool bIsStiffDecelerating;
    bool bWantsToFly;
};
