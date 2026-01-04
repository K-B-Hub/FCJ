// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter/HybridCat.h"
#include "Actor/Objects/HoldingObject.h"
#include "Actor/Objects/Projectile.h"
#include "Components/BoxComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

AHybridCat::AHybridCat()
{
	// BiteCat initialization
	CurrentHeldObject = nullptr;
	bIsCharging = false;
	CurrentChargeTime = 0.0f;

	// AttackCat initialization
	bIsParrying = false;
}

void AHybridCat::BeginPlay()
{
	Super::BeginPlay();
}

void AHybridCat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// BiteCat replication
	DOREPLIFETIME(AHybridCat, CurrentHeldObject);
	DOREPLIFETIME(AHybridCat, bIsCharging);
	DOREPLIFETIME(AHybridCat, CurrentChargeTime);
}

void AHybridCat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 서버에서만 충전 시간 업데이트
	if (HasAuthority() && bIsCharging && IsHoldingObject())
	{
		CurrentChargeTime += DeltaTime;

		// 최대 충전 시간 도달 시 자동 발사
		if (CurrentChargeTime >= MaxChargeTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("Max charge reached - Auto throwing"));
			ServerReleaseThrow(CurrentChargeTime);
		}
	}
}

// ========== Core Overrides with Mutual Exclusion ==========

void AHybridCat::PerformSpecialAction()
{
	// MUTUAL EXCLUSION: 패링 중 잡기 불가
	if (bIsParrying)
	{
		UE_LOG(LogTemp, Warning, TEXT("HybridCat: Cannot grab while parrying"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("HybridCat PerformSpecialAction called"));

	// BiteCat의 특수 행동: 잡기 or 충전 시작
	if (IsHoldingObject())
	{
		// 현재 오브젝트를 들고 있다면 충전 시작
		UE_LOG(LogTemp, Warning, TEXT("Starting charge for throw"));
		StartCharging();
	}
	else
	{
		// 오브젝트를 들고 있지 않다면 가장 가까운 오브젝트 잡기 시도
		AHoldingObject* NearestObject = FindNearestHoldableObject();
		UE_LOG(LogTemp, Warning, TEXT("Found nearest object: %s"), NearestObject ? *NearestObject->GetName() : TEXT("None"));

		if (NearestObject && CanHoldObject(NearestObject))
		{
			UE_LOG(LogTemp, Warning, TEXT("Attempting to hold object"));
			HoldObject(NearestObject);
		}
		else if (NearestObject)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot hold object - Object may already be held or other restriction"));
		}
	}

	// 부모 클래스의 Blueprint 이벤트도 호출
	Super::PerformSpecialAction();
}

void AHybridCat::PerformSecondarySpecialAction()
{
	// MUTUAL EXCLUSION: 물체 소유 중 패링 불가
	if (IsHoldingObject())
	{
		UE_LOG(LogTemp, Warning, TEXT("HybridCat: Cannot parry while holding object"));
		return;
	}

	// AttackCat의 특수 행동: 패링
	if (AttackMontage && !bIsParrying)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (!AnimInstance->IsAnyMontagePlaying())
			{
				AnimInstance->Montage_Play(AttackMontage);
			}
		}
	}

	// 부모 클래스의 Blueprint 이벤트도 호출
	Super::PerformSecondarySpecialAction();
}

void AHybridCat::OnSpecialActionReleased()
{
	// BiteCat 충전 시스템 릴리즈
	if (IsHoldingObject() && bIsCharging)
	{
		ReleaseThrow();
	}

	// 부모 클래스의 Blueprint 이벤트도 호출
	Super::OnSpecialActionReleased();
}

// ========== BiteCat Functions ==========

AHoldingObject* AHybridCat::FindNearestHoldableObject() const
{
	if (!SpecialActionBox)
	{
		return nullptr;
	}

	// SpecialActionBox와 오버랩된 HoldingObject들 가져오기
	TArray<AActor*> OverlappingActors;
	SpecialActionBox->GetOverlappingActors(OverlappingActors, AHoldingObject::StaticClass());

	AHoldingObject* NearestObject = nullptr;
	float MinDistance = FLT_MAX;

	for (AActor* Actor : OverlappingActors)
	{
		if (AHoldingObject* HoldingObject = Cast<AHoldingObject>(Actor))
		{
			if (HoldingObject->CanBeHeld())
			{
				float Distance = FVector::Dist(GetActorLocation(), HoldingObject->GetActorLocation());
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					NearestObject = HoldingObject;
				}
			}
		}
	}

	return NearestObject;
}

bool AHybridCat::CanHoldObject(AHoldingObject* Object) const
{
	if (!Object)
	{
		return false;
	}

	// 이미 다른 오브젝트를 들고 있다면 불가능
	if (IsHoldingObject())
	{
		return false;
	}

	// 오브젝트가 이미 잡혀있다면 불가능
	if (!Object->CanBeHeld())
	{
		return false;
	}

	return true;
}

void AHybridCat::HoldObject(AHoldingObject* Object)
{
	if (!CanHoldObject(Object))
	{
		return;
	}

	// 서버에 요청 전송
	ServerHoldObject(Object);
}

void AHybridCat::ServerHoldObject_Implementation(AHoldingObject* Object)
{
	if (!CanHoldObject(Object))
	{
		return;
	}

	CurrentHeldObject = Object;
	Object->OnHeld(this);

	UE_LOG(LogTemp, Warning, TEXT("HybridCat grabbed object: %s"), *Object->GetName());
}

void AHybridCat::StartCharging()
{
	// 물체를 잡고 있을 때만 충전 가능
	if (!IsHoldingObject())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot charge - not holding an object"));
		return;
	}

	// 서버에 충전 시작 요청
	ServerStartCharging();
}

void AHybridCat::ServerStartCharging_Implementation()
{
	if (!IsHoldingObject())
	{
		return;
	}

	bIsCharging = true;
	CurrentChargeTime = 0.0f;
	UE_LOG(LogTemp, Warning, TEXT("Server: Started charging throw"));
}

void AHybridCat::ReleaseThrow()
{
	if (!IsHoldingObject())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot throw - not holding an object"));
		return;
	}

	if (!bIsCharging)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot throw - not charging"));
		return;
	}

	// 서버에 현재 충전 시간과 함께 던지기 요청
	ServerReleaseThrow(CurrentChargeTime);
}

void AHybridCat::ServerReleaseThrow_Implementation(float ChargeTime)
{
	if (!CurrentHeldObject)
	{
		return;
	}

	if (!bIsCharging)
	{
		return;
	}

	// 충전 시간을 기반으로 던지기 힘 계산
	float ChargeRatio = FMath::Clamp(ChargeTime / MaxChargeTime, 0.0f, 1.0f);
	float CalculatedForce = FMath::Lerp(MinThrowForce, MaxThrowForce, ChargeRatio);

	UE_LOG(LogTemp, Warning, TEXT("Server: Releasing throw with charge time: %.2f, force: %.2f"), ChargeTime, CalculatedForce);

	// 카메라의 전방 방향 계산 (카메라가 없으면 캐릭터 방향 사용)
	UCameraComponent* CameraComp = GetCameraComponent();
	FVector ForwardDirection = CameraComp ? CameraComp->GetForwardVector() : GetActorForwardVector();

	// 던질 방향에 약간의 위쪽 각도 추가 (포물선 궤적을 위해)
	FVector ThrowDirection = ForwardDirection + FVector(0.0f, 0.0f, 0.3f);
	ThrowDirection.Normalize();

	// 캐릭터의 현재 이동 속도 가져오기
	FVector CharacterVelocity = GetVelocity();

	// 오브젝트를 놓기
	AHoldingObject* ObjectToThrow = CurrentHeldObject;
	CurrentHeldObject->OnReleased();
	CurrentHeldObject = nullptr;

	// 충전 상태 초기화
	bIsCharging = false;
	CurrentChargeTime = 0.0f;

	// HoldingObject는 MeshComponent에 물리가 설정되어 있으므로 MeshComponent에 힘을 가함
	if (UStaticMeshComponent* MeshComp = ObjectToThrow->FindComponentByClass<UStaticMeshComponent>())
	{
		if (MeshComp->IsSimulatingPhysics())
		{
			// 계산된 힘으로 임펄스 계산
			FVector ThrowImpulse = ThrowDirection * CalculatedForce * MeshComp->GetMass();

			// 캐릭터의 현재 속도를 운동량(momentum)으로 변환하여 추가
			FVector CharacterMomentum = CharacterVelocity * MeshComp->GetMass();

			// 최종 임펄스 = 던지기 힘 + 캐릭터의 운동량
			FVector FinalImpulse = ThrowImpulse + CharacterMomentum;

			MeshComp->AddImpulse(FinalImpulse);

			UE_LOG(LogTemp, Warning, TEXT("Server: Threw object - CharacterVelocity: %s, ThrowImpulse: %s, FinalImpulse: %s"),
				*CharacterVelocity.ToString(), *ThrowImpulse.ToString(), *FinalImpulse.ToString());
		}
		else
		{
			// 물리 시뮬레이션이 비활성화되어 있다면 활성화하고 던지기
			MeshComp->SetSimulatePhysics(true);

			// 계산된 힘으로 임펄스 계산
			FVector ThrowImpulse = ThrowDirection * CalculatedForce * MeshComp->GetMass();

			// 캐릭터의 현재 속도를 운동량(momentum)으로 변환하여 추가
			FVector CharacterMomentum = CharacterVelocity * MeshComp->GetMass();

			// 최종 임펄스 = 던지기 힘 + 캐릭터의 운동량
			FVector FinalImpulse = ThrowImpulse + CharacterMomentum;

			MeshComp->AddImpulse(FinalImpulse);

			UE_LOG(LogTemp, Warning, TEXT("Server: Threw object (physics enabled) - CharacterVelocity: %s, FinalImpulse: %s"),
				*CharacterVelocity.ToString(), *FinalImpulse.ToString());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Server: Threw object with calculated force: %.2f"), CalculatedForce);
}

float AHybridCat::CalculateThrowForce() const
{
	// 충전 시간을 0~1 범위로 정규화
	float ChargeRatio = FMath::Clamp(CurrentChargeTime / MaxChargeTime, 0.0f, 1.0f);

	// 최소 힘에서 최대 힘으로 선형 보간
	float CalculatedForce = FMath::Lerp(MinThrowForce, MaxThrowForce, ChargeRatio);

	return CalculatedForce;
}

// ========== AttackCat Functions ==========

void AHybridCat::StartParrying()
{
	bIsParrying = true;

	// Start checking for projectiles to reflect
	GetWorld()->GetTimerManager().SetTimer(
		ProjectileReflectionTimer,
		this,
		&AHybridCat::CheckAndReflectProjectiles,
		0.01f, // Check every 0.01 seconds for responsiveness
		true   // Loop
	);
}

void AHybridCat::StopParrying()
{
	bIsParrying = false;

	// Stop checking for projectiles
	GetWorld()->GetTimerManager().ClearTimer(ProjectileReflectionTimer);
}

void AHybridCat::CheckAndReflectProjectiles()
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

void AHybridCat::ReflectProjectile(AProjectile* Projectile)
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

void AHybridCat::PushNearbyObjects()
{
	// 서버에 밀치기 요청
	ServerPushNearbyObjects();
}

void AHybridCat::ServerPushNearbyObjects_Implementation()
{
	if (!SpecialActionBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("HybridCat: SpecialActionBox is null"));
		return;
	}

	// SpecialActionBox와 오버랩된 HoldingObject들 가져오기
	TArray<AActor*> OverlappingActors;
	SpecialActionBox->GetOverlappingActors(OverlappingActors, AHoldingObject::StaticClass());

	if (OverlappingActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("HybridCat: No HoldingObjects in range"));
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

				UE_LOG(LogTemp, Warning, TEXT("HybridCat: Pushed object %s with force %.2f"), *HoldingObject->GetName(), PushForce);
			}
		}
	}
}
