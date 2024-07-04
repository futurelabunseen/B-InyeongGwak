
// TGGameMode.cpp
#include "TGGameMode.h"
#include "Character/TGEnemyBase.h"
#include "Kismet/GameplayStatics.h"

ATGGameMode::ATGGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATGGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentLevel = 1;
    CurrentWaveInterval = InitialWaveInterval;
    CurrentMonsterCount = InitialMonsterCount;
    ElapsedTime = 0.0f;

    InitializeObjectPool();
}

void ATGGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ElapsedTime += DeltaTime;

    if (ElapsedTime >= CurrentWaveInterval)
    {
        SpawnWave();
        ElapsedTime = 0.0f;
    }

    UpdateWaveProgress(DeltaTime);
}

void ATGGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Clean up the object pool
    for (ATGEnemyBase* Monster : MonsterPool)
    {
        if (Monster)
        {
            Monster->Destroy();
        }
    }
    MonsterPool.Empty();

    for (ATGEnemyBase* Monster : ActiveMonsters)
    {
        if (Monster)
        {
            Monster->Destroy();
        }
    }
    ActiveMonsters.Empty();
}

void ATGGameMode::SpawnWave()
{
    UE_LOG(LogTemp, Log, TEXT("Spawning wave %d with %d monsters."), CurrentLevel, CurrentMonsterCount);

    for (int32 i = 0; i < CurrentMonsterCount; ++i)
    {
        ATGEnemyBase* SpawnedMonster = SpawnMonster();
        if (SpawnedMonster)
        {
            ActiveMonsters.Add(SpawnedMonster);
        }
    }

    if (CurrentLevel < 10)
    {
        CurrentLevel++;
        CurrentWaveInterval = FMath::Lerp(InitialWaveInterval, FinalWaveInterval, (float)CurrentLevel / 9);
        CurrentMonsterCount = FMath::Lerp(InitialMonsterCount, FinalMonsterCount, (float)CurrentLevel / 9);
    }
}

ATGEnemyBase* ATGGameMode::SpawnMonster()
{
    ATGEnemyBase* Monster = GetMonsterFromPool();
    if (Monster)
    {
        FVector SpawnLocation = SpawnOrigin + FMath::VRand() * SpawnRadius;
        Monster->SetActorLocation(SpawnLocation);
        Monster->SetActorRotation(FRotator::ZeroRotator);
        Monster->SetActorHiddenInGame(false);
        Monster->SetActorEnableCollision(true);
        Monster->InitializeMonster();
    }
    return Monster;
}

void ATGGameMode::UpdateWaveProgress(float DeltaTime)
{
    OnWaveChanged.Broadcast(CurrentLevel, GetWaveProgress(), 0);
}

int32 ATGGameMode::GetCurrentWaveLevel() const
{
    return CurrentLevel;
}

float ATGGameMode::GetWaveProgress() const
{
    return ElapsedTime / CurrentWaveInterval;
}

void ATGGameMode::InitializeObjectPool()
{
    if (!MonsterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterClass is not set in ATGGameMode!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid World in InitializeObjectPool"));
        return;
    }

    for (int32 i = 0; i < InitialPoolSize; ++i)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ATGEnemyBase* NewMonster = World->SpawnActor<ATGEnemyBase>(MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        
        if (NewMonster)
        {
            NewMonster->SetActorHiddenInGame(true);
            NewMonster->SetActorEnableCollision(false);
            MonsterPool.Add(NewMonster);
        }
    }
}

ATGEnemyBase* ATGGameMode::GetMonsterFromPool()
{
    for (ATGEnemyBase* Monster : MonsterPool)
    {
        if (Monster && Monster->IsHidden())
        {
            MonsterPool.Remove(Monster);
            return Monster;
        }
    }

    // If no available monster in the pool, create a new one
    UWorld* World = GetWorld();
    if (World && MonsterClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ATGEnemyBase* NewMonster = World->SpawnActor<ATGEnemyBase>(MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (NewMonster)
        {
            UE_LOG(LogTemp, Log, TEXT("Created new monster for pool"));
            return NewMonster;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Failed to get or create monster from pool"));
    return nullptr;
}

void ATGGameMode::ReturnMonsterToPool(ATGEnemyBase* Monster)
{
    if (Monster)
    {
        Monster->SetActorHiddenInGame(true);
        Monster->SetActorEnableCollision(false);
        Monster->SetActorLocation(FVector::ZeroVector);
        MonsterPool.Add(Monster);
        ActiveMonsters.Remove(Monster);
    }
}