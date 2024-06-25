#include "TGCustomizingCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameInstance/TGGameInstance.h"
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
			NewMeshComponent ->SetupAttachment(GetMesh());
			NewMeshComponent ->SetLeaderPoseComponent(GetMesh(), true);
			CharacterPartsMap.Add(PartCode, NewMeshComponent);
		}
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
	for (const auto& Elem : MyGameInstance->ModuleBodyPartIndex)
	{
		E_PartsCode PartCode = Elem.Key;
		FName PartName = Elem.Value;
		const FMeshCategoryData* TargetData = MyGameInstance->ModuleDataAsset->BaseMeshComponent.Find(Elem.Value);
		MyGameInstance->ModuleBodyPartIndex[TargetData->Category] = Elem.Value;
		CharacterPartsMap[TargetData->Category]->SetSkeletalMesh(MyGameInstance->ModuleDataAsset->GetMeshByID(Elem.Value));
	}
	
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
