
#include "TGCustomizingPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/ScrollBox.h"
#include "Blueprint/UserWidget.h"
#include "Utility/TGModuleDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Math/Vector.h"
#include "Engine/World.h"
#include "Weapon/TGBaseWeapon.h"
#include "Character/TGCustomizingCharacterBase.h"
#include "GameInstance/TGGameInstance.h"
#include "Kismet/GameplayStatics.h"

ATGCustomizingPlayerController::ATGCustomizingPlayerController()
{
    CurrentState = ECustomizingState::Idle;
    bIsDragging = false;
}

void ATGCustomizingPlayerController::BeginPlay()
{
    Super::BeginPlay();
    MyGameInstance = Cast<UTGCGameInstance>(GetGameInstance());
    if (!MyGameInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast GameInstance to UTGCGameInstance."));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }

    WeaponDataAsset = TWeakObjectPtr<UTGWeaponDataAsset>(MyGameInstance->WeaponDataAsset);
    ModuleDataAsset = TWeakObjectPtr<UTGModuleDataAsset>(MyGameInstance->ModuleDataAsset);

    if (!WeaponDataAsset.IsValid() || !ModuleDataAsset.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("DataAsset is null."));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    ACharacter* MyCharacter = GetCharacter();
    if (MyCharacter)
    {
        MySkeletalMeshComponent = MyCharacter->GetMesh();
        MyCharacter->SetActorEnableCollision(false);
    }
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ATGCustomizingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickLeftMouse);
    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Started, this, &ATGCustomizingPlayerController::MouseLeftClickStarted);
    EnhancedInputComponent->BindAction(LClickAction, ETriggerEvent::Completed, this, &ATGCustomizingPlayerController::MouseLeftClickEnded);
    EnhancedInputComponent->BindAction(RClickAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickRightMouse);
    EnhancedInputComponent->BindAction(EscapeAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnClickEscape);
    EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnRotateAction);
    EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Triggered, this, &ATGCustomizingPlayerController::OnEnterAction);
}

void ATGCustomizingPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        break;
    case ECustomizingState::OnDragActor:
        UpdateWeaponActorPosition();
        CheckWeaponActorProximity();
        break;
    case ECustomizingState::OnSnappedActor:
        CheckSnappedCancellation();
        break;
    case ECustomizingState::OnRotateActor:
        DrawDebugHighlight();
        break;
    }
}

void ATGCustomizingPlayerController::OnRotateAction(const FInputActionValue& Value)
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnRotateActor:
        OnRotateCharacter(Value);
        break;
    }
}

void ATGCustomizingPlayerController::OnRotateCharacter(const FInputActionValue& Value)
{
    FVector2D InputVector = Value.Get<FVector2D>();
    AddYawInput(InputVector.Y);
    if (GetPawn())
    {
        ACharacter* MyCharacter = GetCharacter();
        UCameraComponent* FollowCamera = MyCharacter ? MyCharacter->FindComponentByClass<UCameraComponent>() : nullptr;
        if (FollowCamera)
        {
            float ZoomFactor = 10.0f;
            float NewOrthoWidth = FollowCamera->OrthoWidth + InputVector.X * -ZoomFactor;
            float MinOrthoWidth = 100.0f;
            float MaxOrthoWidth = 1000.0f;
            FollowCamera->OrthoWidth = FMath::Clamp(NewOrthoWidth, MinOrthoWidth, MaxOrthoWidth);
        }
    }
}

void ATGCustomizingPlayerController::OnEnterAction()
{
    MyGameInstance->ChangeLevel(TargetLevelName);
}

void ATGCustomizingPlayerController::OnRotateWeaponActor()
{
    if (!bIsDragging || !CurrentRotationSelectedActor.IsValid())
    {
        return;
    }

    FVector2D MouseCurrentLocation;
    GetMousePosition(MouseCurrentLocation.X, MouseCurrentLocation.Y);

    FVector2D MouseDelta = MouseCurrentLocation - MouseStartLocation;
    MouseStartLocation = MouseCurrentLocation;

    FQuat QuatRotation = FQuat::Identity;

    if (FMath::Abs(MouseDelta.X) > KINDA_SMALL_NUMBER)
    {
        float YawValue = -MouseDelta.X * RotationSpeed * MouseSensitivity;
        QuatRotation *= FQuat(FRotator(0.0f, YawValue, 0.0f));
    }

    if (FMath::Abs(MouseDelta.Y) > KINDA_SMALL_NUMBER)
    {
        float PitchValue = MouseDelta.Y * RotationSpeed * MouseSensitivity;
        QuatRotation *= FQuat(FRotator(0.0f, 0.0f, PitchValue));
    }
    
    if (!QuatRotation.IsIdentity())
    {
        CurrentRotationSelectedActor->AddActorLocalRotation(QuatRotation, false, nullptr, ETeleportType::None);
    }
}

void ATGCustomizingPlayerController::OnClickRightMouse()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        RemoveWeaponInDesiredPosition();
        break;
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnRotateActor:
        ReturnToIdleState();
        break;
    }
}

void ATGCustomizingPlayerController::OnClickLeftMouse()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        TryFindRotatingTargetActor();
        break;
    case ECustomizingState::OnDragActor:
        break;
    case ECustomizingState::OnSnappedActor:
        AttachWeapon();
        break;
    case ECustomizingState::OnRotateActor:
        OnRotateWeaponActor();
        break;
    }
}

void ATGCustomizingPlayerController::MouseLeftClickStarted()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnRotateActor:
        StartMouseDrag();
        break;
    }
}

void ATGCustomizingPlayerController::StartMouseDrag()
{
    bIsDragging = true;
    GetMousePosition(MouseStartLocation.X, MouseStartLocation.Y);
}

void ATGCustomizingPlayerController::MouseLeftClickEnded()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
    case ECustomizingState::OnDragActor:
    case ECustomizingState::OnSnappedActor:
    case ECustomizingState::OnRotateActor:
        StopMouseDrag();
        break;
    }
}

void ATGCustomizingPlayerController::StopMouseDrag()
{
    bIsDragging = false;
}

void ATGCustomizingPlayerController::TryFindRotatingTargetActor()
{
    UE_LOG(LogTemp, Warning, TEXT("Try Find Rotation Target"));
    TWeakObjectPtr<AActor> TargetActor = FindTargetActorUnderMouse();
    if (TargetActor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Found Rotation Target"));
        CurrentRotationSelectedActor = TargetActor;
        CurrentState = ECustomizingState::OnRotateActor;
    }
}



void ATGCustomizingPlayerController::DrawDebugHighlight()
{
    if (CurrentRotationSelectedActor.IsValid())
    {
        FVector ActorLocation = CurrentRotationSelectedActor->GetActorLocation();
        float SphereRadius = 2.0f; 
        FColor SphereColor = FColor::Red; 
        DrawDebugSphere(GetWorld(), ActorLocation, SphereRadius, 12, SphereColor, false, -1.0f, 0, 2.0f);
    }
}

AActor* ATGCustomizingPlayerController::FindTargetActorUnderMouse()
{
    float MouseX, MouseY;
    if (GetMousePosition(MouseX, MouseY))
    {
        FVector WorldLocation, WorldDirection;
        if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        {
            FHitResult Hit;
            FVector Start = WorldLocation;
            FVector End = Start + WorldDirection * 10000;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(GetPawn());
            ECollisionChannel Channel = ECC_Visibility;
            if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Channel, QueryParams))
            {
                if (Hit.GetActor())
                {
                    return Hit.GetActor();
                }
            }
        }
    }
    return nullptr;
}

void ATGCustomizingPlayerController::RemoveWeaponInDesiredPosition()
{
    ATGBaseWeapon* HitWeapon = Cast<ATGBaseWeapon>(FindTargetActorUnderMouse());
    RemoveWeaponFromCharacter(HitWeapon);
}

void ATGCustomizingPlayerController::OnClickEscape()
{
    ReturnToIdleState();
}

void ATGCustomizingPlayerController::UpdateWeaponActorPosition()
{
    if (GetPawn())
    {
            FVector WorldLocation, WorldDirection;
            if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
            {
                FVector PlaneNormal = FVector(1, 0, 0);
                FVector PlanePoint = FVector(0, 0, 100);
                FVector FarPoint = WorldLocation + WorldDirection * 10000;
                FVector IntersectionPoint = FMath::LinePlaneIntersection(WorldLocation, FarPoint, PlanePoint, PlaneNormal);
                if (CurrentSpawnedActor.IsValid())
                {
                    CurrentSpawnedActor->SetActorLocation(IntersectionPoint);
                }
            }
    }
}

void ATGCustomizingPlayerController::RemoveWeaponFromCharacter(ATGBaseWeapon* WeaponToRemove) const
{
    if (WeaponToRemove)
    {
        MyGameInstance->AttachedActorsMap.FindAndRemoveChecked(WeaponToRemove->BoneID);
        WeaponToRemove->Destroy();
        WeaponToRemove = nullptr;
    }
}

void ATGCustomizingPlayerController::CheckWeaponActorProximity()
{
    if (MySkeletalMeshComponent && CurrentSpawnedActor.IsValid())
    {
        FVector TargetLocation = CurrentSpawnedActor->GetActorLocation();
        FVector ClosestBoneLocation;
        FName ClosestBoneName = MySkeletalMeshComponent->FindClosestBone(TargetLocation, &ClosestBoneLocation);
        if (!ClosestBoneName.IsNone())
        {
            float ClosestBoneDistance = FVector::Dist(TargetLocation, ClosestBoneLocation);
            if (ClosestBoneDistance < SnapCheckDistance)
            {
                CurrentState = ECustomizingState::OnSnappedActor;
                CurrentSpawnedActor->SetActorLocation(ClosestBoneLocation);
                CurrentTargetBone = ClosestBoneName;
                UE_LOG(LogTemp, Log, TEXT("Found Spot! Bone :%s"), *ClosestBoneName.ToString());
            }
        }
    }
}

void ATGCustomizingPlayerController::CheckSnappedCancellation()
{
    float MouseX, MouseY;
    if (GetMousePosition(MouseX, MouseY))
    {
        FVector WorldLocation, WorldDirection;
        if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        {
            if (FVector::Distance(CurrentSpawnedActor->GetActorLocation(), WorldLocation) > 10)
            {
                CurrentState = ECustomizingState::OnDragActor;
            }
        }
    }
}

void ATGCustomizingPlayerController::AttachWeapon()
{
    if (!MySkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("SkeletalMeshComponent is not valid."));
        return;
    }

    if (!CurrentSpawnedActor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("CurrentSpawnedActor is not valid or not spawned."));
        return;
    }

    if (!WeaponDataAsset.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponDataAsset is null."));
        return;
    }
    

    AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(CurrentSpawnedActor->GetClass(), MySkeletalMeshComponent->GetComponentLocation(), FRotator::ZeroRotator);
    ClonedActor->AttachToComponent(MySkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, CurrentTargetBone);
    ClonedActor->SetActorEnableCollision(true);
    ATGBaseWeapon* ClonedWeaponActor = Cast<ATGBaseWeapon>(ClonedActor);
    ATGBaseWeapon* OriginalWeaponActor = Cast<ATGBaseWeapon>(CurrentSpawnedActor);
    if (!OriginalWeaponActor || !ClonedWeaponActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast into weapon class."));
        return;
    }
    FName TempWeaponID = OriginalWeaponActor->WeaponID;
    ClonedWeaponActor->WeaponID = TempWeaponID;
    ClonedWeaponActor->BoneID = CurrentTargetBone;
    FAttachedActorData TempData(TempWeaponID, ClonedActor->GetActorRotation());
    MyGameInstance->AttachedActorsMap.Add(CurrentTargetBone, TempData);
    if (!ClonedActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn cloned actor."));
        return;
    }
    ReturnToIdleState();
}

void ATGCustomizingPlayerController::ReturnToIdleState()
{
    switch (CurrentState)
    {
    case ECustomizingState::Idle:
        break;
    case ECustomizingState::OnDragActor:
        break;
    case ECustomizingState::OnSnappedActor:
        break;
    case ECustomizingState::OnRotateActor:
        SaveRotationData();
        break;
    }
    ResetHoldingData();
    CurrentState = ECustomizingState::Idle;
}

void ATGCustomizingPlayerController::SaveRotationData() const
{
    ATGBaseWeapon* WeaponActor = Cast<ATGBaseWeapon>(CurrentRotationSelectedActor);
    if (WeaponActor)
    {
        UE_LOG(LogTemp, Log, TEXT("target BoneID : %s"), *WeaponActor->BoneID.ToString());
        FName TargetID = WeaponActor->BoneID;
        FAttachedActorData* FoundActorData = MyGameInstance->AttachedActorsMap.Find(TargetID);
        if (FoundActorData)
        {
            FoundActorData->Rotation = CurrentRotationSelectedActor->GetActorRotation();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Actor data not found for TargetID: %s"), *TargetID.ToString());
        }
    }
}

void ATGCustomizingPlayerController::ResetHoldingData()
{
    if (CurrentSpawnedActor.IsValid())
    {
        CurrentSpawnedActor->Destroy();
        CurrentTargetBone = FName();
        CurrentSpawnedActor = nullptr;
        CurrentRotationSelectedActor = nullptr;
    }
}

void ATGCustomizingPlayerController::AddButtonToPanel(UScrollBox* TargetPanel, TSubclassOf<UUserWidget> TargetButtonWidget, FName ID)
{
    ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetCharacter());
    if (MyCharacter && MyCharacter->CustomizingComponent)
    {
        MyCharacter->CustomizingComponent->AddButtonToPanel(TargetPanel, TargetButtonWidget, ID);
    }
}

void ATGCustomizingPlayerController::AddWeaponButtonToPanel(UScrollBox* TargetPanel)
{
    if (!WeaponDataAsset.Get())
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponDataAsset is null."));
        return;
    }

    ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetCharacter());
    if (MyCharacter && MyCharacter->CustomizingComponent)
    {
        MyCharacter->CustomizingComponent->AddWeaponButtonToPanel(TargetPanel, WeaponDataAsset.Get());
    }
}

void ATGCustomizingPlayerController::AddModuleButtonToPanel(UScrollBox* TargetPanel)
{
    if (!ModuleDataAsset.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ModuleDataAsset is null."));
        return;
    }

    ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetCharacter());
    if (MyCharacter && MyCharacter->CustomizingComponent)
    {
        MyCharacter->CustomizingComponent->AddModuleButtonToPanel(TargetPanel, ModuleDataAsset.Get());
    }
}

void ATGCustomizingPlayerController::OnWeaponSelected(FName WeaponID)
{
    if (!WeaponDataAsset.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponDataAsset is null."));
        return;
    }

    ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetCharacter());
    if (MyCharacter && MyCharacter->CustomizingComponent)
    {
        if (CurrentSpawnedActor.IsValid())
        {
            CurrentSpawnedActor->Destroy();
        }

        UBlueprintGeneratedClass* WeaponClass = MyCharacter->CustomizingComponent->GetWeaponClassById(WeaponID, WeaponDataAsset.Get());
        if (WeaponClass)
        {
            FActorSpawnParameters SpawnParameters;
            SpawnParameters.Owner = GetPawn();
            SpawnParameters.Instigator = GetPawn()->GetInstigator();

            CurrentSpawnedActor = GetWorld()->SpawnActor<AActor>(WeaponClass, GetPawn()->GetActorLocation(), GetPawn()->GetActorRotation(), SpawnParameters);
            CurrentSpawnedActor->SetActorEnableCollision(false);
            ATGBaseWeapon* WeaponActor = Cast<ATGBaseWeapon>(CurrentSpawnedActor);
            if (!WeaponActor)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to cast into weapon class."));
                return;
            }
            WeaponActor->WeaponID = WeaponID;

            if (!CurrentSpawnedActor.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn weapon."));
                return;
            }

            CurrentState = ECustomizingState::OnDragActor;
        }
    }
}

void ATGCustomizingPlayerController::OnModuleSelected(FName WeaponID)
{
    if (!ModuleDataAsset.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ModuleDataAsset is null."));
        return;
    }

    ATGCustomizingCharacterBase* MyCharacter = Cast<ATGCustomizingCharacterBase>(GetCharacter());
    if (MyCharacter && MyCharacter->CustomizingComponent)
    {
        const FMeshCategoryData* TargetData = ModuleDataAsset->BaseMeshComponent.Find(WeaponID);
        MyGameInstance->ModuleBodyPartIndex[TargetData->Category] = WeaponID;
        UClass* AnimClass = MyCharacter->GetMesh()->GetAnimClass();
        USkeleton* Skeleton = MyCharacter->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton();
        USkeletalMesh* MergedMesh = MyCharacter->CustomizingComponent->GetMergedCharacterParts(MyGameInstance->ModuleBodyPartIndex, ModuleDataAsset.Get());
        if (MergedMesh == nullptr)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to merge Mesh."));
            return;
        }
        MergedMesh->USkeletalMesh::SetSkeleton(Skeleton);
        MyCharacter->GetMesh()->SetSkeletalMesh(MergedMesh);
        MyCharacter->GetMesh()->SetAnimInstanceClass(AnimClass);
    }
}