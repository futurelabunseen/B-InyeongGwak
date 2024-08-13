#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Utility/TGModuleSystem.h"
#include "TGGameInstance.generated.h"


UCLASS()
class TOPGUN_API UTGCGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UTGCGameInstance();
	virtual void Init() override;
	void ResetGame();
	void ChangeLevel(FName LevelName) const;
	virtual void Shutdown() override;
	TMap<E_PartsCode, FName> ModuleBodyPartIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
	class UDataTable* EquipDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Assets")
	class UTGModuleDataAsset* ModuleDataAsset;
	
	UPROPERTY(BlueprintReadWrite, Category = "Score")
	int32 PlayerScore;
	
	UFUNCTION()
	void OnMonsterDeath();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UTGEquipmentManager* GetEquipmentManager() const { return EquipmentManager; }
	
private:
	UPROPERTY()
	UTGEquipmentManager* EquipmentManager;

};
