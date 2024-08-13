#include "TGGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/TGEquipmentManager.h"
#include "Utility/TGModuleSystem.h"

UTGCGameInstance::UTGCGameInstance()
{
	EquipmentManager = CreateDefaultSubobject<UTGEquipmentManager>(TEXT("EquipmentManager"));
}

void UTGCGameInstance::Init()
{
	Super::Init();
	EquipmentManager->SetEquipDataAsset(EquipDataTable);
	
	ModuleBodyPartIndex.Add(E_PartsCode::Head, FName("HeadA"));
	ModuleBodyPartIndex.Add(E_PartsCode::UpperBody, FName("BodyA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LowerBody, FName("LegA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LeftHand, FName("LeftArmA"));
	ModuleBodyPartIndex.Add(E_PartsCode::RightHand, FName("RightArmA"));

	PlayerScore = 0; // Initialize score
}

void UTGCGameInstance::ResetGame()
{
	ModuleBodyPartIndex.Empty();
	EquipmentManager->EmptyEquipMap();
	ModuleBodyPartIndex.Add(E_PartsCode::Head, FName("HeadA"));
	ModuleBodyPartIndex.Add(E_PartsCode::UpperBody, FName("BodyA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LowerBody, FName("LegA"));
	ModuleBodyPartIndex.Add(E_PartsCode::LeftHand, FName("LeftArmA"));
	ModuleBodyPartIndex.Add(E_PartsCode::RightHand, FName("RightArmA"));

	PlayerScore = 0;
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

void UTGCGameInstance::OnMonsterDeath()
{
	PlayerScore += 10; 
	UE_LOG(LogTemp, Log, TEXT("Player Score: %d"), PlayerScore);
}






