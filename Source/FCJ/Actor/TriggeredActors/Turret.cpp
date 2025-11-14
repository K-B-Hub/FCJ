// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredActors/Turret.h"
#include "PlayerCharacter/CatBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;

	// 레플리케이션 설정
	bReplicates = true;

	// 베이스 메시
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BaseMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	RootComponent = BaseMesh;

	// 포탑 헤드 (회전용)
	TurretHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretHead"));
	TurretHead->SetupAttachment(BaseMesh);
	TurretHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 감지 영역
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(BaseMesh);
	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	DetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	DetectionSphere->SetGenerateOverlapEvents(true);

	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ATurret::OnDetectionBeginOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ATurret::OnDetectionEndOverlap);

	bIsActive = false;
	CurrentTarget = nullptr;
}

void ATurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATurret, DetectedPlayers);
	DOREPLIFETIME(ATurret, CurrentTarget);
	DOREPLIFETIME(ATurret, bIsActive);
}

void ATurret::BeginPlay()
{
	Super::BeginPlay();

	if (!ProjectileClass)
	{
		ProjectileClass = AProjectile::StaticClass();
	}

	// 감지 영역 반경 업데이트
	DetectionSphere->SetSphereRadius(DetectionRadius);
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//CleanupDetectedPlayers();

	if (bIsActive && CurrentTarget)
	{
		UpdateTargetRotation(DeltaTime);
	}
}

void ATurret::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
		return;

	ACatBase* Cat = Cast<ACatBase>(OtherActor);
	if (Cat && !DetectedPlayers.Contains(Cat))
	{
		DetectedPlayers.Add(Cat);
	}
}

void ATurret::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	if (!HasAuthority())
		return;

	ACatBase* Cat = Cast<ACatBase>(OtherActor);
	if (Cat)
	{
		DetectedPlayers.Remove(Cat);

		if (CurrentTarget == Cat)
		{
			CurrentTarget = nullptr;
		}
	}
}

void ATurret::SetActive(bool bNewActive)
{
	// 서버에서만 상태 변경 가능 (리플리케이션을 통해 클라이언트 동기화)
	if (!HasAuthority())
	{
		return;
	}

	if (bIsActive == bNewActive)
		return;

	bIsActive = bNewActive;

	if (bIsActive)
	{
		StartFiring();
	}
	else
	{
		StopFiring();
	}
}

void ATurret::StartFiring()
{
	if (!GetWorld() || !HasAuthority())
		return;

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &ATurret::FireProjectile, FireInterval, true);
}

void ATurret::StopFiring()
{
	if (!GetWorld())
		return;

	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	CurrentTarget = nullptr;
}

void ATurret::UpdateTargetRotation(float DeltaTime)
{
	if (!CurrentTarget || !TurretHead)
		return;

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector TurretLocation = TurretHead->GetComponentLocation();
	FVector Direction = (TargetLocation - TurretLocation).GetSafeNormal();

	// Yaw만 회전 (수평 회전만)
	FRotator TargetRotation = Direction.Rotation();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;

	FRotator CurrentRotation = TurretHead->GetComponentRotation();
	FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);

	TurretHead->SetWorldRotation(NewRotation);
}

void ATurret::SelectRandomTarget()
{
	if (DetectedPlayers.Num() == 0)
	{
		CurrentTarget = nullptr;
		return;
	}

	int32 RandomIndex = FMath::RandRange(0, DetectedPlayers.Num() - 1);
	CurrentTarget = DetectedPlayers[RandomIndex];
}

void ATurret::FireProjectile()
{
	if (!GetWorld() || !ProjectileClass || !HasAuthority())
		return;

	if (DetectedPlayers.Num() == 0)
		return;

	// 랜덤 타겟 선택
	SelectRandomTarget();

	if (!CurrentTarget)
		return;

	// 발사 위치 및 방향 계산
	FVector SpawnLocation = TurretHead->GetComponentLocation() + TurretHead->GetForwardVector() * ProjectileSpawnOffset.X;
	SpawnLocation += TurretHead->GetRightVector() * ProjectileSpawnOffset.Y;
	SpawnLocation += TurretHead->GetUpVector() * ProjectileSpawnOffset.Z;

	FVector Direction = (CurrentTarget->GetActorLocation() - SpawnLocation).GetSafeNormal();
	EProjectileType ProjectileType = GetRandomProjectileType();

	// 발사체 생성 (서버에서만 생성, 자동으로 클라이언트에 리플리케이트됨)
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AProjectile* NewProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, Direction.Rotation(), SpawnParams);
	if (NewProjectile)
	{
		NewProjectile->InitializeProjectile(ProjectileType, CurrentTarget, Direction);
		MulticastOnProjectileFired(NewProjectile);
	}
}

void ATurret::MulticastOnProjectileFired_Implementation(AProjectile* NewProjectile)
{
	// 클라이언트에서 발사 시각/사운드 효과 처리
}

EProjectileType ATurret::GetRandomProjectileType() const
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

void ATurret::CleanupDetectedPlayers()
{
	DetectedPlayers.RemoveAll([](ACatBase* Cat)
	{
		return !IsValid(Cat);
	});
}

