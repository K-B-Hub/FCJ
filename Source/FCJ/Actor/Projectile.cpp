// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Projectile.h"
#include "PlayerCharacter/CatBase.h"
#include "Actor/ProjectileVolume.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 레플리케이션 설정
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(5.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	CurrentLifeTime = 0.0f;
	TargetCharacter = nullptr;
	SpawnVolume = nullptr;
	SpawnLocation = FVector::ZeroVector;

	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
}

void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectile, ProjectileType);
	DOREPLIFETIME(AProjectile, TargetCharacter);
	DOREPLIFETIME(AProjectile, SpawnVolume);
	DOREPLIFETIME(AProjectile, bIsParried);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);
	if (MeshComponent)
	{
		MeshComponent->SetIsReplicated(true);
	}
	if (CollisionComponent)
	{
		CollisionComponent->SetIsReplicated(true);
	}
	
	SetLifeSpan(LifeTime);
	SpawnLocation = GetActorLocation();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentLifeTime += DeltaTime;
	
	// Check distance from spawn volume
	if (SpawnVolume && IsValid(SpawnVolume))
	{
		float DistanceFromVolume = FVector::Dist(GetActorLocation(), SpawnVolume->GetActorLocation());
		if (DistanceFromVolume > MaxDistanceFromVolume)
		{
			Destroy();
			return;
		}
	}
	else if (SpawnLocation != FVector::ZeroVector)
	{
		// Fallback: check distance from spawn location if volume is not available
		float DistanceFromSpawn = FVector::Dist(GetActorLocation(), SpawnLocation);
		if (DistanceFromSpawn > MaxDistanceFromVolume)
		{
			Destroy();
			return;
		}
	}
	
	UpdateMovement(DeltaTime);
}

void AProjectile::InitializeProjectile(EProjectileType InProjectileType, ACatBase* InTarget, FVector InDirection)
{
	ServerInitializeProjectile(InProjectileType, InTarget, InDirection);
}

void AProjectile::ServerInitializeProjectile_Implementation(EProjectileType InProjectileType, ACatBase* InTarget, FVector InDirection)
{
	ProjectileType = InProjectileType;
	TargetCharacter = InTarget;
	InitialDirection = InDirection.IsNormalized() ? InDirection : InDirection.GetSafeNormal();

	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;

	switch (ProjectileType)
	{
	case EProjectileType::Straight:
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->Velocity = InitialDirection * ProjectileSpeed;
		break;

	case EProjectileType::Homing:
		if (TargetCharacter)
		{
			ProjectileMovement->bIsHomingProjectile = true;
			ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
			ProjectileMovement->HomingTargetComponent = TargetCharacter->GetRootComponent();
		}
		else
		{
			ProjectileMovement->bIsHomingProjectile = false;
			ProjectileMovement->Velocity = InitialDirection * ProjectileSpeed;
		}
		break;

	case EProjectileType::SlightGuided:
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->Velocity = InitialDirection * ProjectileSpeed;
		break;
	}

	MulticastInitializeProjectile(InProjectileType, InTarget, InDirection);
}

void AProjectile::MulticastInitializeProjectile_Implementation(EProjectileType InProjectileType, ACatBase* InTarget, FVector InDirection)
{
	if (!HasAuthority())
	{
		ProjectileType = InProjectileType;
		TargetCharacter = InTarget;
		InitialDirection = InDirection.IsNormalized() ? InDirection : InDirection.GetSafeNormal();

		ProjectileMovement->InitialSpeed = ProjectileSpeed;
		ProjectileMovement->MaxSpeed = ProjectileSpeed;

		switch (ProjectileType)
		{
		case EProjectileType::Straight:
			ProjectileMovement->bIsHomingProjectile = false;
			ProjectileMovement->Velocity = InitialDirection * ProjectileSpeed;
			break;

		case EProjectileType::Homing:
			if (TargetCharacter)
			{
				ProjectileMovement->bIsHomingProjectile = true;
				ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
				ProjectileMovement->HomingTargetComponent = TargetCharacter->GetRootComponent();
			}
			else
			{
				ProjectileMovement->bIsHomingProjectile = false;
				ProjectileMovement->Velocity = InitialDirection * ProjectileSpeed;
			}
			break;

		case EProjectileType::SlightGuided:
			ProjectileMovement->bIsHomingProjectile = false;
			ProjectileMovement->Velocity = InitialDirection * ProjectileSpeed;
			break;
		}
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HasAuthority() && OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		ServerOnHit(OtherActor, Hit);
	}
}

void AProjectile::ServerOnHit_Implementation(AActor* OtherActor, const FHitResult& Hit)
{
	ACatBase* HitCat = Cast<ACatBase>(OtherActor);
	if (HitCat)
	{
		// Apply damage
		UGameplayStatics::ApplyPointDamage(HitCat, Damage, GetActorLocation(), Hit, nullptr, this, UDamageType::StaticClass());

		// Apply knockback force
		FVector KnockbackDirection = (HitCat->GetActorLocation() - GetActorLocation()).GetSafeNormal();

		// Apply the knockback force to the character's movement component
		if (UCharacterMovementComponent* MovementComponent = HitCat->GetCharacterMovement())
		{
			// Calculate horizontal knockback
			FVector HorizontalKnockback = FVector(KnockbackDirection.X, KnockbackDirection.Y, 0.0f).GetSafeNormal() * KnockbackForce;

			// Add vertical knockback component
			FVector VerticalKnockback = FVector(0.0f, 0.0f, VerticalKnockbackForce);

			FVector TotalKnockback = HorizontalKnockback + VerticalKnockback;
			MovementComponent->AddImpulse(TotalKnockback, true);
		}
	}

	MulticastOnHit(OtherActor, Hit);
	Destroy();
}

void AProjectile::MulticastOnHit_Implementation(AActor* OtherActor, const FHitResult& Hit)
{
	// 클라이언트에서 파티클 이펙트나 사운드 재생 등 시각적 효과 처리
	// 실제 데미지나 넉백은 서버에서만 처리됨
}

void AProjectile::UpdateMovement(float DeltaTime)
{
	switch (ProjectileType)
	{
	case EProjectileType::Homing:
		HandleHomingMovement(DeltaTime);
		break;

	case EProjectileType::SlightGuided:
		HandleSlightGuidedMovement(DeltaTime);
		break;

	case EProjectileType::Straight:
	default:
		break;
	}
}

void AProjectile::HandleHomingMovement(float DeltaTime)
{
	if (!TargetCharacter || !IsValid(TargetCharacter) || bIsParried)
	{
		ProjectileMovement->bIsHomingProjectile = false;
		return;
	}
}

void AProjectile::HandleSlightGuidedMovement(float DeltaTime)
{
	if (!TargetCharacter || !IsValid(TargetCharacter) || bIsParried)
	{
		return;
	}

	FVector CurrentVelocity = ProjectileMovement->Velocity;
	FVector DirectionToTarget = (TargetCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	
	FVector GuidedVelocity = FMath::VInterpTo(CurrentVelocity.GetSafeNormal(), DirectionToTarget, DeltaTime, SlightGuidanceStrength / 1000.0f);
	ProjectileMovement->Velocity = GuidedVelocity * ProjectileSpeed;
}

void AProjectile::SetSpawnVolume(AProjectileVolume* Volume)
{
	SpawnVolume = Volume;
}

