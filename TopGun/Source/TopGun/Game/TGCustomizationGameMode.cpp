// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/TGCustomizationGameMode.h"
#include "Player/TGCustomizingPlayerController.h"
#include "Utility/TGCustomizationHandlingManager.h"
#include "Utility/TGCustomizingStateManager.h"

ATGCustomizationGameMode::ATGCustomizationGameMode()
{
}

void ATGCustomizationGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("PostLogin"));
    UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(GetGameInstance());
    if (!GameInstance)
    {
       UE_LOG(LogTemp, Error, TEXT("GameInstance is not of type UTGCGameInstance"));
       return;
    }

    
    ATGCustomizingPlayerController* CustomizingPlayerController = Cast<ATGCustomizingPlayerController>(NewPlayer);
    if (CustomizingPlayerController)
    {
       StateManager = NewObject<UTGCustomizingStateManager>(CustomizingPlayerController);


       CustomizingStateInterface = Cast<ITGCustomizingInterface>(StateManager);
       CustomizingPlayerInterface = Cast<ITGCustomizingPlayerInterface>(CustomizingPlayerController);

       CustomizingComponent = NewObject<UTGCustomizationHandlingManager>(CustomizingPlayerController);
       
       CustomizingUIManager = NewObject<UTGCustomizingUIManager>(CustomizingPlayerController);
       CustomizingUIManager->Initialize(CustomizingComponent, CustomizingPlayerController->GetWorld());
       
       if (StateManager && CustomizingStateInterface && CustomizingPlayerInterface)
       {
          StateManager->SetPlayerController(CustomizingPlayerInterface, CustomizingComponent);
          CustomizingPlayerController->SetCustomizingStateManager(CustomizingStateInterface, CustomizingComponent, CustomizingUIManager);
          UE_LOG(LogTemp, Log, TEXT("Successfully set up CustomizingStateManager"));
       }
       else
       {
          if (!StateManager)
             UE_LOG(LogTemp, Error, TEXT("Failed to create UTGCustomizingStateManager"));
          if (!CustomizingStateInterface)
             UE_LOG(LogTemp, Error, TEXT("UTGCustomizingStateManager does not implement ITGCustomizingInterface"));
          if (!CustomizingPlayerInterface)
             UE_LOG(LogTemp, Error, TEXT("CustomizingPlayerController does not implement ITGCustomizingPlayerInterface"));
       }
    }
    else
    {
       UE_LOG(LogTemp, Error, TEXT("Failed to cast to ATGCustomizingPlayerController"));
    }
}