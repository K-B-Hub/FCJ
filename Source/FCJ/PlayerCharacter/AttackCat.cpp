// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter/AttackCat.h"
#include "Actor/Projectile.h"
#include "Components/BoxComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAttackCat::AAttackCat()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAttackCat::BeginPlay()
{
	Super::BeginPlay();
}

void AAttackCat::PerformSpecialAction()
{
	if (AttackMontage && !bIsParrying && !IsPlayingMontage())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(AttackMontage);
			SetMontageePlaying(true);
		}
	}
	
	Super::PerformSpecialAction();
}

void AAttackCat::StartParrying()
{
	bIsParrying = true;
	
	// Start checking for projectiles to reflect
	GetWorld()->GetTimerManager().SetTimer(
		ProjectileReflectionTimer,
		this,
		&AAttackCat::CheckAndReflectProjectiles,
		0.01f, // Check every 0.01 seconds for responsiveness
		true   // Loop
	);
}

void AAttackCat::StopParrying()
{
	bIsParrying = false;
	
	// Stop checking for projectiles
	GetWorld()->GetTimerManager().ClearTimer(ProjectileReflectionTimer);
	
	// Check if attack montage is finished and reset montage state
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (!AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			SetMontageePlaying(false);
		}
	}
}

void AAttackCat::CheckAndReflectProjectiles()
{
	if (!bIsParrying || !SpecialActionBox)
	{
		return;
	}

	// Get all overlapping actors
	TArray<AActor*> OverlappingActors;
	SpecialActionBox->GetOverlappingActors(OverlappingActors, AProjectile::StaticClass());

	// Reflect each projectile
	for (AActor* Actor : OverlappingActors)
	{
		if (AProjectile* Projectile = Cast<AProjectile>(Actor))
		{
			ReflectProjectile(Projectile);
		}
	}
}

void AAttackCat::ReflectProjectile(AProjectile* Projectile)
{
	if (!Projectile)
	{
		return;
	}

	// Mark projectile as parried to disable homing
	Projectile->SetParried(true);

	// Get projectile movement component
	UProjectileMovementComponent* ProjectileMovement = Projectile->FindComponentByClass<UProjectileMovementComponent>();
	if (!ProjectileMovement)
	{
		return;
	}

	// Disable homing for parried projectiles
	ProjectileMovement->bIsHomingProjectile = false;

	// Reflect the velocity (reverse direction)
	FVector CurrentVelocity = ProjectileMovement->Velocity;
	FVector ReflectedVelocity = -CurrentVelocity;
	
	// Set the reflected velocity
	ProjectileMovement->Velocity = ReflectedVelocity;
	
	// Also update the projectile's forward direction
	Projectile->SetActorRotation(ReflectedVelocity.Rotation());
}

