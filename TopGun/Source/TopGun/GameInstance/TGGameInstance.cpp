// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/TGGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "Utility/TGModuleSystem.h"

void UTGCGameInstance::Init()
{
	Super::Init();
	ModuleBodyPartIndex.Add(E_PartsCode::Head, FName("HeadA"));
	ModuleBodyPartIndex.Add(E_PartsCode::UpperBody, FName("BodyA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LowerBody, FName("LegA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LeftHand, FName("LeftArmA"));
	ModuleBodyPartIndex.Add(E_PartsCode::RightHand, FName("RightArmA"));
}

void UTGCGameInstance::ChangeLevel(FName LevelName) const
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetLevelName is not set."));
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::OpenLevel(World, LevelName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GetWorld() returned null."));
	}
}

void UTGCGameInstance::Shutdown()
{
	Super::Shutdown();
}