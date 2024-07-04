#include "TGCustomizingCharacterBase.h"
#include "Armour/TGBaseArmour.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameInstance/TGGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TGStatWidget.h"
#include "Utility/TGArmoursDataAsset.h"
#include "Utility/TGCustomizingComponent.h"
#include "Utility/TGModuleDataAsset.h"
#include "Weapon/TGBaseWeapon.h"

ATGCustomizingCharacterBase::ATGCustomizingCharacterBase()
{
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    CustomizingComponent = CreateDefaultSubobject<UTGCustomizingComponent>(TEXT("CustomizingComponent"));

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
        UE_LOG(LogTemp, Error, TEXT("StatWidget Setting up."));
        MyGameInstance->OnChangeStat.AddUObject(StatWidget, &UTGStatWidget::UpdateStatBar);
        MyGameInstance->BroadcastTotalStats();        
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Not a stat Widget."));
    }
}

void ATGCustomizingCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    MyGameInstance = Cast<UTGCGameInstance>(GetGameInstance());
    if (!MyGameInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast GameInstance to UTGCGameInstance."));
        FGenericPlatformMisc::RequestExit(false);
        return;
    }
    SetupPlayerModel(GetMesh());
}

void ATGCustomizingCharacterBase::SetupPlayerModel(USkeletalMeshComponent* TargetMesh) const
{
    if (!MyGameInstance.IsValid() || !MyGameInstance->ModuleDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance or ModuleDataAsset is invalid."));
        return;
    }

    SetupCharacterParts();
    SetupWeapons(TargetMesh);
    SetupArmour(TargetMesh);
}

void ATGCustomizingCharacterBase::SetupCharacterParts() const
{
    for (const auto& Elem : MyGameInstance->ModuleBodyPartIndex)
    {
        E_PartsCode PartCode = Elem.Key;
        FName PartName = Elem.Value;
        const FMeshCategoryData* TargetData = MyGameInstance->ModuleDataAsset->BaseMeshComponent.Find(PartName);

        if (!TargetData)
        {
            UE_LOG(LogTemp, Error, TEXT("TargetData is null for PartName: %s"), *PartName.ToString());
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
                UE_LOG(LogTemp, Error, TEXT("SkeletalMesh is null for PartName: %s"), *PartName.ToString());
            }
        }
    }
}

void ATGCustomizingCharacterBase::ResetGameCustomization()
{
    MyGameInstance->ResetGame();
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void ATGCustomizingCharacterBase::SetupWeapons(USkeletalMeshComponent* TargetMesh) const
{
    if (!MyGameInstance || !MyGameInstance->WeaponDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance or WeaponDataAsset is invalid"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get World"));
        return;
    }

    const TMap<FName, FAttachedActorData>& WeaponActorMap = MyGameInstance->GetEntireWeaponActorMap();

    for (const auto& Elem : WeaponActorMap)
    {
        const FName& BoneID = Elem.Key;
        const FName& WeaponID = Elem.Value.ActorID;

        if (!SetupSingleWeapon(World, TargetMesh, WeaponID, BoneID, Elem.Value.Rotation))
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to setup weapon: %s"), *WeaponID.ToString());
        }
    }
}

bool ATGCustomizingCharacterBase::SetupSingleWeapon(UWorld* World, USkeletalMeshComponent* TargetMesh, const FName& WeaponID, const FName& BoneID, const FRotator& Rotation) const
{
    if (!MyGameInstance->WeaponDataAsset->BaseWeaponClasses.Contains(WeaponID))
    {
        UE_LOG(LogTemp, Error, TEXT("BaseWeaponClass is invalid for WeaponID: %s"), *WeaponID.ToString());
        return false;
    }

    UClass* WeaponClass = *MyGameInstance->WeaponDataAsset->BaseWeaponClasses.Find(WeaponID);
    if (!WeaponClass)
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponClass is null for WeaponID: %s"), *WeaponID.ToString());
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* ClonedActor = World->SpawnActor<AActor>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (!ClonedActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn actor for WeaponClass: %s"), *WeaponClass->GetName());
        return false;
    }

    ClonedActor->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneID);
    ClonedActor->SetActorRotation(Rotation);
    ClonedActor->SetActorEnableCollision(true);

    ATGBaseWeapon* WeaponActor = Cast<ATGBaseWeapon>(ClonedActor);
    if (!WeaponActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast to ATGBaseWeapon for actor: %s"), *ClonedActor->GetName());
        ClonedActor->Destroy();
        return false;
    }

    WeaponActor->WeaponID = WeaponID;
    WeaponActor->BoneID = BoneID;

    return true;
}

void ATGCustomizingCharacterBase::SetupArmour(USkeletalMeshComponent* TargetMesh) const
{
    for (const auto& Elem : MyGameInstance->GetEntireArmourActorMap())
    {
        FName BoneID = Elem.Key;
        FName ArmourID = Elem.Value.ActorID;

        if (!MyGameInstance->ArmourDataAsset || !MyGameInstance->ArmourDataAsset->BaseArmourClass.Contains(ArmourID))
        {
            UE_LOG(LogTemp, Error, TEXT("ArmourDataAsset or BaseArmourClass is invalid for ArmourID: %s"), *ArmourID.ToString());
            continue;
        }

        UBlueprintGeneratedClass* ArmourClass = *MyGameInstance->ArmourDataAsset->BaseArmourClass.Find(ArmourID);
        if (!ArmourClass)
        {
            UE_LOG(LogTemp, Error, TEXT("ArmourClass is null for ArmourID: %s"), *ArmourID.ToString());
            continue;
        }

        AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(ArmourClass, FVector::ZeroVector, FRotator::ZeroRotator);
        if (!ClonedActor)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn actor for ArmourClass: %s"), *ArmourClass->GetName());
            continue;
        }

        ClonedActor->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneID);
        ClonedActor->SetActorRotation(Elem.Value.Rotation);
        ClonedActor->SetActorEnableCollision(true);

        ATGBaseArmour* ArmourActor = Cast<ATGBaseArmour>(ClonedActor);
        if (!ArmourActor)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to cast to ATGBaseArmour for actor: %s"), *ClonedActor->GetName());
            ClonedActor->Destroy(); // Cleanup if cast fails
            continue;
        }

        ArmourActor->ArmourID = ArmourID;
        ArmourActor->BoneID = BoneID;
    }
}

void ATGCustomizingCharacterBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}
