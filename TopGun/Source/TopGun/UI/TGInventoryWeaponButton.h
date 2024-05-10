#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "TGInventoryWeaponButton.generated.h"

UCLASS()
class TOPGUN_API UTGInventoryWeaponButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetupButton(FName WeaponName);
	UPROPERTY(BlueprintReadOnly)
	FName WeaponID;
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;
};
