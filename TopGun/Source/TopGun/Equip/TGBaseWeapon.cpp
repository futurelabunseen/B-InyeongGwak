#include "TGBaseWeapon.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/TGEnemyBase.h"
#include "Engine/DamageEvents.h"
#include "GameInstance/TGGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Data/EquipmentData.h"
#include "Utility/TGEquipmentManager.h"

ATGBaseWeapon::ATGBaseWeapon()
{
    PrimaryActorTick.bCanEverTick = true;
    AttackDamage = 0;
    Attack = 0;
    Category = ETGEquipmentCategory::Weapon;
}

FVector ATGBaseWeapon::GetArrowForwardVector_Implementation() const
{
    const UArrowComponent* ArrowComponent = FindComponentByClass<UArrowComponent>();

    if (ArrowComponent != nullptr)
    {
        const FVector ForwardStartLocation = ArrowComponent->GetComponentLocation();
        const FVector EndLocation = ForwardStartLocation + (ArrowComponent->GetForwardVector() * 20000.0f);
        return EndLocation - ForwardStartLocation;
    }
    else
    {
        return GetActorForwardVector();
    }
}

FQuat ATGBaseWeapon::GetAimingRotation_Implementation(const FVector& TargetVector) const
{
    FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetVector);
    return LookRotation.Quaternion();
}



void ATGBaseWeapon::InitializeWeapon(FName PWeaponID, FName PBoneID)
{
    SetDefaultRotation_Implementation();
}

void ATGBaseWeapon::CheckForHitScan(bool bIsAiming)
{
  UArrowComponent* ArrowComponent = FindComponentByClass<UArrowComponent>();
    if (ArrowComponent != nullptr)
    {
        ACharacter* PlayerActor = UGameplayStatics::GetPlayerCharacter(GetWorld(),0);
        const FVector StartLocation = bIsAiming==true?(PlayerActor->GetActorLocation()+FVector(0, 0, 70)):ArrowComponent->GetComponentLocation();
        const FVector EndLocation = StartLocation + (ArrowComponent->GetForwardVector() * 1000.0f);

        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        
        FHitResult OutHitResult;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);
        Params.AddIgnoredActor(PlayerActor);
        if (bool HitDetected = GetWorld()->LineTraceSingleByObjectType(OutHitResult, StartLocation, EndLocation, ObjectParams, Params))
        {
            ATGCharacterBase* HitEnemy = Cast<ATGCharacterBase>(OutHitResult.GetActor());
            if (HitEnemy)
            {
                UE_LOG(LogTemp, Log, TEXT("Weapon %s Hit Actor : %s"), *this->GetActorNameOrLabel(), *OutHitResult.GetActor()->GetActorNameOrLabel());
                FVector KnockbackDirection = OutHitResult.ImpactPoint - GetActorLocation();
                KnockbackDirection.Normalize();
                FDamageEvent DamageEvent;
                HitEnemy->TakeDamage(AttackDamage, DamageEvent, GetWorld()->GetFirstPlayerController(), this);
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("Weapon %s Hit Actor : %s"), *this->GetActorNameOrLabel(), *OutHitResult.GetActor()->GetActorNameOrLabel());
                DrawDebugLine(GetWorld(), StartLocation, OutHitResult.Location, FColor::Yellow, false, 2.0f, 0, 0.5f);
                DrawDebugSphere(GetWorld(), OutHitResult.GetActor()->GetActorLocation(), 10.0f, 12, FColor::Yellow, false, 2.0f);
            }
        } else
        {
            UE_LOG(LogTemp, Log, TEXT("Weapon %s Hit Actor : no result"), *this->GetActorNameOrLabel());
            DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 0.5f);
        }

    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("ArrowComponent Null"));
    }
}

void ATGBaseWeapon::SetDefaultRotation_Implementation()
{
    if (UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(GetGameInstance()))
    {
        FAttachedActorData FoundActorData;
        const FEquipmentKey TargetKey(BoneID,Category, EquipmentID);
        if (GameInstance->GetEquipmentManager()->GetEquipActorData(TargetKey, FoundActorData))
        {
            DefaultRotationQuat = FQuat(FoundActorData.Rotation);
        }
        else
        {
            DefaultRotationQuat = GetActorQuat();
            UE_LOG(LogTemp, Log, TEXT("WeaponBase: Couldn't find data for BoneID: %s"), *ATGBaseWeapon::GetBoneID_Implementation().ToString());
        }
    }
    else
    {
        DefaultRotationQuat = GetActorQuat();
        UE_LOG(LogTemp, Log, TEXT("WeaponBase: Failed to get game instance"));
    }
}

void ATGBaseWeapon::SetRotation_Implementation(const FQuat& QuatRotation)
{
    SetActorRotation(QuatRotation);
}


FName ATGBaseWeapon::GetEquipmentID_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("GetEquipmentID_Implementation : EquipID: %s"), *EquipmentID.ToString());
    return EquipmentID;
}

FName ATGBaseWeapon::GetBoneID_Implementation()
{
    return BoneID;
}

void ATGBaseWeapon::SetEquipmentID_Implementation(FName NewEquipmentID)
{
    UE_LOG(LogTemp, Warning, TEXT("SetEquipmentID_Implementation : EquipID: %s"), *NewEquipmentID.ToString());
    EquipmentID = NewEquipmentID;
}

void ATGBaseWeapon::SetBoneID_Implementation(FName NewBoneID)
{
    BoneID = NewBoneID;
}

void ATGBaseWeapon::InitializeEquipment_Implementation(FName NewEquipmentID, FName NewBoneID)
{
    EquipmentID = NewEquipmentID;
    BoneID = NewBoneID;
}

USpringArmComponent* ATGBaseWeapon::GetSpringArmComponent_Implementation() const
{
    return MySpringArmComponent;
}

void ATGBaseWeapon::SetSpringArmComponent_Implementation(USpringArmComponent* SpringArmComponent)
{
    MySpringArmComponent = SpringArmComponent;
}

void ATGBaseWeapon::BeginPlay()
{
    Super::BeginPlay();
}

ETGEquipmentCategory ATGBaseWeapon::GetCategory_Implementation()
{
    return Category;
}


