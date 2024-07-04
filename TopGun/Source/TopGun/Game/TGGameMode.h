#pragma once

#include "CoreMinimal.h"
#include "Character/TGEnemyBase.h"
#include "GameFramework/GameModeBase.h"
#include "TGGameMode.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnChangedWave, int, float, int);

UCLASS()
class TOPGUN_API ATGGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ATGGameMode();
    FOnChangedWave OnWaveChanged;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "Wave System")
    int32 GetCurrentWaveLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Wave System")
    float GetWaveProgress() const;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Wave System")
    TSubclassOf<ATGEnemyBase> MonsterClass;

    UPROPERTY(EditDefaultsOnly, Category = "Wave System")
    float InitialWaveInterval = 60.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Wave System")
    float FinalWaveInterval = 30.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Wave System")
    int32 InitialMonsterCount = 5;

    UPROPERTY(EditDefaultsOnly, Category = "Wave System")
    int32 FinalMonsterCount = 20;

    UPROPERTY(EditDefaultsOnly, Category = "Wave System")
    FVector SpawnOrigin = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, Category = "Wave System")
    float SpawnRadius = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Object Pool")
    int32 InitialPoolSize = 50;

private:
    void SpawnWave();
    ATGEnemyBase* SpawnMonster();
    void UpdateWaveProgress(float DeltaTime);
    void InitializeObjectPool();
    ATGEnemyBase* GetMonsterFromPool();
    void ReturnMonsterToPool(ATGEnemyBase* Monster);

    int32 CurrentLevel;
    float CurrentWaveInterval;
    int32 CurrentMonsterCount;
    float ElapsedTime;

    UPROPERTY()
    TArray<ATGEnemyBase*> MonsterPool;

    UPROPERTY()
    TArray<ATGEnemyBase*> ActiveMonsters;
};