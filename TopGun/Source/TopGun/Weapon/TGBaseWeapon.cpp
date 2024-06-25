#include "TGBaseWeapon.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/TGEnemyBase.h"
#include "Engine/DamageEvents.h"
#include "GameInstance/TGGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ATGBaseWeapon::ATGBaseWeapon()
{
    PrimaryActorTick.bCanEverTick = true;
    AttackDamage = 20.0f;
}

FVector ATGBaseWeapon::GetArrowForwardVector() const
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

FQuat ATGBaseWeapon::GetAimingRotation(const FVector& TargetVector) const
{
    FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetVector);
    return LookRotation.Quaternion();
}

void ATGBaseWeapon::SetWeaponID_Implementation(FName InWeaponID)
{
    this->WeaponID = InWeaponID;
}

void ATGBaseWeapon::SetBoneID_Implementation(FName InBoneID)
{
    this->BoneID = InBoneID;
}

FName ATGBaseWeapon::GetWeaponID_Implementation()
{
    return WeaponID;
}

FName ATGBaseWeapon::GetBoneID_Implementation()
{
    return BoneID;
}

FQuat ATGBaseWeapon::GetDefaultRoationQuat() const
{
    return DefaultRotationQuat;
}

void ATGBaseWeapon::SetSpringArmComponent(USpringArmComponent* SpringArmComponent)
{
    MySpringArmComponent = SpringArmComponent;
}

USpringArmComponent* ATGBaseWeapon::GetSpringArmComponent() const
{
    return MySpringArmComponent;
}


void ATGBaseWeapon::InitializeWeapon(FName PWeaponID, FName PBoneID)
{
    WeaponID = PWeaponID;
    BoneID = PBoneID;
    SetDefaultRotation();
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

void ATGBaseWeapon::SetDefaultRotation()
{
    if (UTGCGameInstance* GameInstance = Cast<UTGCGameInstance>(GetGameInstance()))
    {
        if (const FAttachedActorData* FoundActorData = GameInstance->AttachedActorsMap.Find(BoneID))
        {
            DefaultRotationQuat = FQuat(FoundActorData->Rotation);
        }
        else
        {
            DefaultRotationQuat = GetActorQuat();
            UE_LOG(LogTemp, Log, TEXT("WeaponBase : Couldn't find with key "));
        }
    }
    else
    {
        DefaultRotationQuat = GetActorQuat();
        UE_LOG(LogTemp, Log, TEXT("WeaponBase : Didn't get default rotation"));
    }
}

void ATGBaseWeapon::SetRotation(const FQuat& QuatRotation)
{
    SetActorRotation(QuatRotation);
}
