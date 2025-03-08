#include "TGCustomizingCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameInstance/TGGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TGStatWidget.h"
#include "Utility/TGModuleDataAsset.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "HAL/IConsoleManager.h"
#include "Interface/TGArmourInterface.h"
#include "Utility/TGEquipmentManager.h"

ATGCustomizingCharacterBase::ATGCustomizingCharacterBase()
{
    // Setup Camera Boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll  = false;
    CameraBoom->bInheritYaw   = false;
    
    // Setup Follow Camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    
    // Setup Character Parts (각 부위에 대한 SkeletalMeshComponent 생성)
    for (int32 i = 0; i < StaticEnum<E_PartsCode>()->NumEnums() - 1; ++i)
    {
        E_PartsCode PartCode = static_cast<E_PartsCode>(i);
        FName PartName = StaticEnum<E_PartsCode>()->GetNameByIndex(i);
        USkeletalMeshComponent* NewMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(*PartName.ToString());
        if (NewMeshComponent)
        {
            NewMeshComponent->SetupAttachment(GetMesh());
            NewMeshComponent->SetLeaderPoseComponent(GetMesh(), true);
            CharacterPartsMap.Add(PartCode, NewMeshComponent);
        }
    }
}

void ATGCustomizingCharacterBase::SetupCharacterWidget(UTGUserWidget* InUserWidget)
{
    UTGStatWidget* StatWidget = Cast<UTGStatWidget>(InUserWidget);
    if (StatWidget)
    {
        UE_LOG(LogTemp, Log, TEXT("StatWidget Setting up."));
        MyGameInstance->GetEquipmentManager()->OnChangeStat.AddUObject(StatWidget, &UTGStatWidget::UpdateStatBar);
        MyGameInstance->GetEquipmentManager()->BroadcastTotalStats();        
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SetupCharacterWidget: Not a stat Widget."));
    }
}

void ATGCustomizingCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    MyGameInstance = Cast<UTGCGameInstance>(GetGameInstance());
    if (!MyGameInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("BeginPlay: Failed to cast GameInstance to UTGCGameInstance."));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }
    
    SetupPlayerModel(GetMesh());
}

void ATGCustomizingCharacterBase::SetupPlayerModel(USkeletalMeshComponent* TargetMesh) const
{
    if (!MyGameInstance.IsValid() || !MyGameInstance->ModuleDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("SetupPlayerModel: Invalid GameInstance or ModuleDataAsset."));
        return;
    }
    
    SetupCharacterParts();
    SetupEquip(TargetMesh);
}

void ATGCustomizingCharacterBase::SetupCharacterParts() const
{
    // 각 부위에 대해 설정된 ModuleBodyPartIndex 기반으로 SkeletalMesh 교체
    for (const auto& Elem : MyGameInstance->ModuleBodyPartIndex)
    {
        E_PartsCode PartCode = Elem.Key;
        FName PartName = Elem.Value;
        const FMeshCategoryData* TargetData = MyGameInstance->ModuleDataAsset->BaseMeshComponent.Find(PartName);

        if (!TargetData)
        {
            UE_LOG(LogTemp, Error, TEXT("SetupCharacterParts: TargetData is null for PartName: %s"), *PartName.ToString());
            continue;
        }

        if (CharacterPartsMap.Contains(TargetData->Category))
        {
            USkeletalMesh* SkeletalMesh = MyGameInstance->ModuleDataAsset->GetMeshByID(PartName);
            if (SkeletalMesh)
            {
                CharacterPartsMap[TargetData->Category]->SetSkeletalMesh(SkeletalMesh);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("SetupCharacterParts: SkeletalMesh is null for PartName: %s"), *PartName.ToString());
            }
        }
    }
}

void ATGCustomizingCharacterBase::SetupEquip(USkeletalMeshComponent* TargetMesh) const
{
    // 모든 장비 데이터를 순회하며 장비 스폰 시도
    for (const auto& Elem : MyGameInstance->GetEquipmentManager()->GetEntireAttachedEquipActorMap())
    {
        if (!SpawnEquip(TargetMesh, Elem.Key.ActorID, Elem.Key.BoneID, Elem.Value.Rotation))
        {
            UE_LOG(LogTemp, Warning, TEXT("SetupEquip: Failed to setup equip: %s"), *Elem.Key.ActorID.ToString());
        }
    }
}

bool ATGCustomizingCharacterBase::SpawnEquip(USkeletalMeshComponent* TargetMesh, const FName& EquipID, const FName& BoneID, const FRotator& Rotation) const
{
    UBlueprintGeneratedClass* EquipClass = MyGameInstance->GetEquipmentManager()->GetEquipClassByID(EquipID);
    if (!EquipClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEquip: EquipClass is null for Equipment %s"), *EquipID.ToString());
        return false;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(EquipClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (!ClonedActor)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEquip: Failed to spawn actor for Equipment %s"), *EquipID.ToString());
        return false;
    }
    
    ClonedActor->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneID);
    ClonedActor->SetActorRotation(Rotation);
    ClonedActor->SetActorEnableCollision(true);
    
    if (!ClonedActor->Implements<UTGBaseEquipmentInterface>())
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnEquip: Actor %s does not implement Equipment Interface"), *ClonedActor->GetName());
        ClonedActor->Destroy();
        FPlatformMisc::RequestExit(false);
        return false;
    }
    
    ITGBaseEquipmentInterface::Execute_SetEquipmentID(ClonedActor, EquipID);
    ITGBaseEquipmentInterface::Execute_SetBoneID(ClonedActor, BoneID);
    
    return true;
}

void ATGCustomizingCharacterBase::ResetGameCustomization()
{
    MyGameInstance->ResetGame();
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void ATGCustomizingCharacterBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}
