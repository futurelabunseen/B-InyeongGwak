#include "TGBaseWeapon.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/TGEnemyBase.h"
#include "Engine/DamageEvents.h"
#include "GameInstance/TGGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Physics/TGCollision.h"

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
    //DrawDebugLine(GetWorld(), GetActorLocation(), TargetVector * 1000.0f, FColor::Blue, false, 5.0f, 0, 1.0f);
    
    return LookRotation.Quaternion();
}

void ATGBaseWeapon::InitializeWeapon(FName PWeaponID, FName PBoneID)
{
    WeaponID = PWeaponID;
    BoneID = PBoneID;
    SetDefaultRotation();
}

void ATGBaseWeapon::CheckForHitScan()
{
    UArrowComponent* ArrowComponent = FindComponentByClass<UArrowComponent>();
    if (ArrowComponent != nullptr)
    {
        const FVector StartLocation = ArrowComponent->GetComponentLocation();
        const FVector EndLocation = StartLocation + (ArrowComponent->GetForwardVector() * 20000.0f);

        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

        FHitResult OutHitResult;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);
        Params.AddIgnoredActor(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
        
        if (bool HitDetected = GetWorld()->LineTraceSingleByObjectType(OutHitResult, StartLocation, EndLocation, ObjectParams, Params))
        {
            ATGCharacterBase* HitEnemy = Cast<ATGCharacterBase>(OutHitResult.GetActor());
            if (HitEnemy)
            {
                FVector KnockbackDirection = OutHitResult.ImpactPoint - GetActorLocation();
                KnockbackDirection.Normalize();
                FDamageEvent DamageEvent;
                HitEnemy->TakeDamage(AttackDamage, DamageEvent, GetWorld()->GetFirstPlayerController(), this);
                UE_LOG(LogTemp, Log, TEXT("Hit Enemy"));
                DrawDebugLine(GetWorld(), StartLocation, OutHitResult.Location, FColor::Green, false, 2.0f, 0, 0.5f);
                UE_LOG(LogTemp, Warning, TEXT("HIT : ENEMY"));

            }
            else
            {
                DrawDebugLine(GetWorld(), StartLocation, OutHitResult.Location, FColor::Yellow, false, 2.0f, 0, 0.5f);
                DrawDebugSphere(GetWorld(), OutHitResult.GetActor()->GetActorLocation(), 10.0f, 12, FColor::Yellow, false, 2.0f);
                UE_LOG(LogTemp, Warning, TEXT("HIT : NONENEMY"));

            }
        } else
        {
            DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 0.5f);
            DrawDebugSphere(GetWorld(), EndLocation, 10.0f, 12, FColor::Red, false, 2.0f);
            UE_LOG(LogTemp, Warning, TEXT("HIT : NOHIT"));

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
