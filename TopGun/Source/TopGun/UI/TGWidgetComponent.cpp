// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TGWidgetComponent.h"

#include "TGUserWidget.h"

void UTGWidgetComponent::InitWidget()
{
	Super::InitWidget();

	UTGUserWidget* ABUserWidget = Cast<UTGUserWidget>(GetWidget());
	if (ABUserWidget)
	{
		ABUserWidget->SetOwningActor(GetOwner());
	}
}
