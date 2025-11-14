// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter/AttackCat.h"
#include "Actor/Objects/Projectile.h"
#include "Actor/Objects/HoldingObject.h"
#include "Components/BoxComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "TimerManager.h"

AAttackCat::AAttackCat()
{
	
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

void AAttackCat::PushNearbyObjects()
{
	// 서버에 밀치기 요청
	ServerPushNearbyObjects();
}

void AAttackCat::ServerPushNearbyObjects_Implementation()
{
	if (!SpecialActionBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackCat: SpecialActionBox is null"));
		return;
	}

	// SpecialActionBox와 오버랩된 HoldingObject들 가져오기
	TArray<AActor*> OverlappingActors;
	SpecialActionBox->GetOverlappingActors(OverlappingActors, AHoldingObject::StaticClass());

	if (OverlappingActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackCat: No HoldingObjects in range"));
		return;
	}

	// 캐릭터의 전방 방향 계산
	FVector ForwardDirection = GetActorForwardVector();

	// 약간의 위쪽 각도 추가 (물체가 날아가는 효과)
	FVector PushDirection = ForwardDirection + FVector(0.0f, 0.0f, 0.2f);
	PushDirection.Normalize();

	// 각 HoldingObject에 힘 가하기
	for (AActor* Actor : OverlappingActors)
	{
		if (AHoldingObject* HoldingObject = Cast<AHoldingObject>(Actor))
		{
			// MeshComponent 찾기 (실제 물리 담당)
			if (UStaticMeshComponent* MeshComp = HoldingObject->FindComponentByClass<UStaticMeshComponent>())
			{
				// 물리 시뮬레이션 확인 및 활성화
				if (!MeshComp->IsSimulatingPhysics())
				{
					MeshComp->SetSimulatePhysics(true);
				}

				// 임펄스로 밀어내기
				FVector PushImpulse = PushDirection * PushForce * MeshComp->GetMass();
				MeshComp->AddImpulse(PushImpulse);

				UE_LOG(LogTemp, Warning, TEXT("AttackCat: Pushed object %s with force %.2f"), *HoldingObject->GetName(), PushForce);
			}
		}
	}
}

