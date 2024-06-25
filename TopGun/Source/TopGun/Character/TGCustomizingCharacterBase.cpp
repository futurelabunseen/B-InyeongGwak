#include "TGCustomizingCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameInstance/TGGameInstance.h"
#include "Utility/TGCustomizingComponent.h"
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
	UClass* AnimClass = TargetMesh->GetAnimClass();
	USkeleton* Skeleton = TargetMesh->GetSkeletalMeshAsset()->GetSkeleton();
	USkeletalMesh* MergedMesh = UTGModuleSystem::GetMergeCharacterParts(MyGameInstance->ModuleBodyPartIndex, MyGameInstance->ModuleDataAsset);
	if (MergedMesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to merge Mesh."));
		return;
	}
	MergedMesh->USkeletalMesh::SetSkeleton(Skeleton);
	TargetMesh->SetSkeletalMesh(MergedMesh);
	TargetMesh->SetAnimInstanceClass(AnimClass);
	
	for (const auto& Elem: MyGameInstance->AttachedActorsMap)
	{
		FName BoneID = Elem.Key;
		FName WeaponID = Elem.Value.ActorID;
		UBlueprintGeneratedClass* WeaponClass = *MyGameInstance->WeaponDataAsset->BaseWeaponClasses.Find(WeaponID);
		AActor* ClonedActor = GetWorld()->SpawnActor<AActor>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator);
		ClonedActor->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, BoneID);
		ClonedActor->SetActorRotation(Elem.Value.Rotation);
		ClonedActor->SetActorEnableCollision(true);
		ATGBaseWeapon* WeaponActor = Cast<ATGBaseWeapon>(ClonedActor);
		if (!WeaponActor)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to cast into weapon class."));
			return;
		}
		WeaponActor->WeaponID = WeaponID;
		WeaponActor->BoneID = BoneID;
	}
}

void ATGCustomizingCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}
