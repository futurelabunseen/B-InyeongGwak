// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABPartsBox.generated.h"

UCLASS()
class ARENABATTLE_API AABPartsBox : public AActor
{
	GENERATED_BODY()
	
public:	
	AABPartsBox();

protected:
	UPROPERTY(VisibleAnywhere, Category = Parts)
	TObjectPtr<class UBoxComponent> Trigger;

	UPROPERTY(VisibleAnywhere, Category = Parts)
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = Effect)
	TObjectPtr<class UParticleSystemComponent> Effect;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	void OnEffectFinished(class UParticleSystemComponent* ParticleSystem);

};
