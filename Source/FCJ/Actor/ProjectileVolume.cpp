// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/ProjectileVolume.h"
#include "PlayerCharacter/CatBase.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

AProjectileVolume::AProjectileVolume()
{
	PrimaryActorTick.bCanEverTick = true;

	// 레플리케이션 설정
	bReplicates = true;

	VolumeComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("VolumeComponent"));
	VolumeComponent->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
	VolumeComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VolumeComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	VolumeComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	VolumeComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	VolumeComponent->SetGenerateOverlapEvents(true);
	RootComponent = VolumeComponent;

	VolumeComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectileVolume::OnVolumeBeginOverlap);
	VolumeComponent->OnComponentEndOverlap.AddDynamic(this, &AProjectileVolume::OnVolumeEndOverlap);

	bIsActive = false;
}

void AProjectileVolume::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectileVolume, OverlappingCats);
	DOREPLIFETIME(AProjectileVolume, ActiveProjectiles);
	DOREPLIFETIME(AProjectileVolume, bIsActive);
}

void AProjectileVolume::BeginPlay()
{
	Super::BeginPlay();
	
	if (!ProjectileClass)
	{
		ProjectileClass = AProjectile::StaticClass();
	}
}

void AProjectileVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CleanupActiveProjectiles();
}

void AProjectileVolume::OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACatBase* Cat = Cast<ACatBase>(OtherActor);
	if (Cat && !OverlappingCats.Contains(Cat))
	{
		OverlappingCats.Add(Cat);
		
		if (bActivateOnOverlap && !bIsActive)
		{
			SetActive(true);
		}
	}
}

void AProjectileVolume::OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	ACatBase* Cat = Cast<ACatBase>(OtherActor);
	if (Cat)
	{
		OverlappingCats.Remove(Cat);
		
		if (bActivateOnOverlap && OverlappingCats.Num() == 0)
		{
			SetActive(false);
		}
	}
}

void AProjectileVolume::SetActive(bool bNewActive)
{
	if (HasAuthority())
	{
		ServerSetActive(bNewActive);
	}
	else
	{
		ServerSetActive(bNewActive);
	}
}

void AProjectileVolume::ServerSetActive_Implementation(bool bNewActive)
{
	if (bIsActive == bNewActive)
		return;

	bIsActive = bNewActive;

	if (bIsActive)
	{
		StartLaunching();
	}
	else
	{
		StopLaunching();
	}
}

void AProjectileVolume::StartLaunching()
{
	if (!GetWorld() || !HasAuthority())
		return;

	GetWorld()->GetTimerManager().SetTimer(LaunchTimerHandle, this, &AProjectileVolume::ServerLaunchProjectile, LaunchInterval, true);
}

void AProjectileVolume::StopLaunching()
{
	if (!GetWorld())
		return;

	GetWorld()->GetTimerManager().ClearTimer(LaunchTimerHandle);
}

void AProjectileVolume::LaunchProjectile()
{
	// 이 함수는 이제 사용하지 않음 - ServerLaunchProjectile로 대체
}

void AProjectileVolume::ServerLaunchProjectile_Implementation()
{
	if (!GetWorld() || !ProjectileClass || !HasAuthority())
		return;

	if (ActiveProjectiles.Num() >= MaxSimultaneousProjectiles)
		return;

	FVector LaunchLocation;
	FVector LaunchDirection;
	EProjectileType ProjectileType = EProjectileType::Straight;
	ACatBase* TargetCat = nullptr;

	bool bShouldTarget = false;

	switch (LaunchMode)
	{
	case EProjectileLaunchMode::TargetedOnly:
		bShouldTarget = true;
		break;
	case EProjectileLaunchMode::RandomOnly:
		bShouldTarget = false;
		break;
	case EProjectileLaunchMode::Mixed:
		bShouldTarget = FMath::RandRange(0.0f, 1.0f) <= TargetedProjectileChance;
		break;
	}

	if (bShouldTarget && OverlappingCats.Num() > 0)
	{
		TargetCat = GetRandomOverlappingCat();
		if (TargetCat)
		{
			LaunchLocation = GetLaunchPointTowards(TargetCat->GetActorLocation());
			LaunchDirection = GetValidLaunchDirection(LaunchLocation, TargetCat->GetActorLocation());
		}
		else
		{
			LaunchLocation = GetRandomLaunchPoint();
			FVector VolumeCenter = GetActorLocation();
			LaunchDirection = GetValidLaunchDirection(LaunchLocation, VolumeCenter);
		}
	}
	else
	{
		LaunchLocation = GetRandomLaunchPoint();
		FVector VolumeCenter = GetActorLocation();
		LaunchDirection = GetValidLaunchDirection(LaunchLocation, VolumeCenter);
	}

	ProjectileType = GetRandomProjectileType();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AProjectile* NewProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, LaunchLocation, LaunchDirection.Rotation(), SpawnParams);
	if (NewProjectile)
	{
		NewProjectile->SetSpawnVolume(this);
		NewProjectile->InitializeProjectile(ProjectileType, TargetCat, LaunchDirection);
		ActiveProjectiles.Add(NewProjectile);
		MulticastOnProjectileLaunched(NewProjectile);
	}
}

void AProjectileVolume::MulticastOnProjectileLaunched_Implementation(AProjectile* NewProjectile)
{
	// 클라이언트에서 발사체 생성 알림 및 시각/사운드 효과 처리
	// 실제 발사체는 이미 서버에서 생성되어 레플리케이션됨
}

FVector AProjectileVolume::GetRandomLaunchPoint() const
{
	FVector VolumeCenter = GetActorLocation();
	FVector VolumeExtent = VolumeComponent->GetScaledBoxExtent();

	int32 Face = FMath::RandRange(0, 4);
	FVector LaunchPoint;

	switch (Face)
	{
	case 0:
		LaunchPoint = FVector(
			VolumeCenter.X + VolumeExtent.X + ProjectileSpawnDistance,
			VolumeCenter.Y + FMath::RandRange(-VolumeExtent.Y, VolumeExtent.Y),
			VolumeCenter.Z + FMath::RandRange(-VolumeExtent.Z, VolumeExtent.Z)
		);
		break;
	case 1:
		LaunchPoint = FVector(
			VolumeCenter.X - VolumeExtent.X - ProjectileSpawnDistance,
			VolumeCenter.Y + FMath::RandRange(-VolumeExtent.Y, VolumeExtent.Y),
			VolumeCenter.Z + FMath::RandRange(-VolumeExtent.Z, VolumeExtent.Z)
		);
		break;
	case 2:
		LaunchPoint = FVector(
			VolumeCenter.X + FMath::RandRange(-VolumeExtent.X, VolumeExtent.X),
			VolumeCenter.Y + VolumeExtent.Y + ProjectileSpawnDistance,
			VolumeCenter.Z + FMath::RandRange(-VolumeExtent.Z, VolumeExtent.Z)
		);
		break;
	case 3:
		LaunchPoint = FVector(
			VolumeCenter.X + FMath::RandRange(-VolumeExtent.X, VolumeExtent.X),
			VolumeCenter.Y - VolumeExtent.Y - ProjectileSpawnDistance,
			VolumeCenter.Z + FMath::RandRange(-VolumeExtent.Z, VolumeExtent.Z)
		);
		break;
	case 4:
		LaunchPoint = FVector(
			VolumeCenter.X + FMath::RandRange(-VolumeExtent.X, VolumeExtent.X),
			VolumeCenter.Y + FMath::RandRange(-VolumeExtent.Y, VolumeExtent.Y),
			VolumeCenter.Z + VolumeExtent.Z + ProjectileSpawnDistance
		);
		break;
	}

	return LaunchPoint;
}

FVector AProjectileVolume::GetLaunchPointTowards(const FVector& TargetLocation) const
{
	FVector VolumeCenter = GetActorLocation();
	FVector VolumeExtent = VolumeComponent->GetScaledBoxExtent();
	FVector DirectionToTarget = (TargetLocation - VolumeCenter).GetSafeNormal();

	FVector LaunchPoint;

	if (FMath::Abs(DirectionToTarget.X) > FMath::Abs(DirectionToTarget.Y))
	{
		if (DirectionToTarget.X > 0)
		{
			LaunchPoint = FVector(
				VolumeCenter.X - VolumeExtent.X - ProjectileSpawnDistance,
				VolumeCenter.Y + FMath::RandRange(-VolumeExtent.Y * 0.8f, VolumeExtent.Y * 0.8f),
				VolumeCenter.Z + FMath::RandRange(-VolumeExtent.Z * 0.8f, VolumeExtent.Z * 0.8f)
			);
		}
		else
		{
			LaunchPoint = FVector(
				VolumeCenter.X + VolumeExtent.X + ProjectileSpawnDistance,
				VolumeCenter.Y + FMath::RandRange(-VolumeExtent.Y * 0.8f, VolumeExtent.Y * 0.8f),
				VolumeCenter.Z + FMath::RandRange(-VolumeExtent.Z * 0.8f, VolumeExtent.Z * 0.8f)
			);
		}
	}
	else
	{
		if (DirectionToTarget.Y > 0)
		{
			LaunchPoint = FVector(
				VolumeCenter.X + FMath::RandRange(-VolumeExtent.X * 0.8f, VolumeExtent.X * 0.8f),
				VolumeCenter.Y - VolumeExtent.Y - ProjectileSpawnDistance,
				VolumeCenter.Z + FMath::RandRange(-VolumeExtent.Z * 0.8f, VolumeExtent.Z * 0.8f)
			);
		}
		else
		{
			LaunchPoint = FVector(
				VolumeCenter.X + FMath::RandRange(-VolumeExtent.X * 0.8f, VolumeExtent.X * 0.8f),
				VolumeCenter.Y + VolumeExtent.Y + ProjectileSpawnDistance,
				VolumeCenter.Z + FMath::RandRange(-VolumeExtent.Z * 0.8f, VolumeExtent.Z * 0.8f)
			);
		}
	}

	return LaunchPoint;
}

FVector AProjectileVolume::GetValidLaunchDirection(const FVector& LaunchPoint, const FVector& TargetLocation) const
{
	FVector VolumeCenter = GetActorLocation();
	FVector ToTarget = (TargetLocation - LaunchPoint).GetSafeNormal();
	FVector ToVolumeCenter = (VolumeCenter - LaunchPoint).GetSafeNormal();
	
	float DotProduct = FVector::DotProduct(ToTarget, ToVolumeCenter);
	
	if (DotProduct < 0.0f)
	{
		FVector RandomDirection = FVector(
			FMath::RandRange(-1.0f, 1.0f),
			FMath::RandRange(-1.0f, 1.0f),
			FMath::RandRange(-0.3f, 0.3f)
		).GetSafeNormal();
		
		FVector AdjustedDirection = FMath::VInterpNormalRotationTo(RandomDirection, ToVolumeCenter, 1.0f, 0.7f);
		return AdjustedDirection.GetSafeNormal();
	}
	
	return ToTarget;
}

EProjectileType AProjectileVolume::GetRandomProjectileType() const
{
	float RandomValue = FMath::RandRange(0.0f, 1.0f);
	
	if (RandomValue <= HomingProjectileChance)
	{
		return EProjectileType::Homing;
	}
	else if (RandomValue <= HomingProjectileChance + SlightGuidedProjectileChance)
	{
		return EProjectileType::SlightGuided;
	}
	else
	{
		return EProjectileType::Straight;
	}
}

ACatBase* AProjectileVolume::GetRandomOverlappingCat() const
{
	if (OverlappingCats.Num() == 0)
		return nullptr;

	int32 RandomIndex = FMath::RandRange(0, OverlappingCats.Num() - 1);
	return OverlappingCats[RandomIndex];
}

void AProjectileVolume::CleanupActiveProjectiles()
{
	ActiveProjectiles.RemoveAll([](AProjectile* Projectile)
	{
		return !IsValid(Projectile);
	});
}

