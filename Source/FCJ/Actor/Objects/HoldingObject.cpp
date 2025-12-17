// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Objects/HoldingObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter/BiteCat.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

AHoldingObject::AHoldingObject()
{
	PrimaryActorTick.bCanEverTick = true;

	// 레플리케이션 설정
	bReplicates = true;

	// Create mesh component (RootComponent, 실제 물리 담당)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	// 메시 컴포넌트에 물리 댐핑 설정
	MeshComponent->SetLinearDamping(LinearDamping);
	MeshComponent->SetAngularDamping(AngularDamping);
	MeshComponent->SetEnableGravity(true);

	// Create collision component (트리거 전용)
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetSphereRadius(SphereRadius);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	// Initialize holding state
	bIsBeingHeld = false;
	HoldingCat = nullptr;
}

void AHoldingObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHoldingObject, bIsBeingHeld);
	DOREPLIFETIME(AHoldingObject, HoldingCat);
}


void AHoldingObject::BeginPlay()
{
	Super::BeginPlay();

	// 시작 위치 저장
	InitialLocation = GetActorLocation();

	//Replicate 설정
	SetReplicateMovement(true);
	if (MeshComponent)
	{
		MeshComponent->SetIsReplicated(true);
	}
	if (CollisionComponent)
	{
		CollisionComponent->SetIsReplicated(true);
		CollisionComponent->SetSphereRadius(SphereRadius);
	}

	// Apply blueprint settings to mesh component (실제 물리 담당)
	if (MeshComponent)
	{
		MeshComponent->SetLinearDamping(LinearDamping);
		MeshComponent->SetAngularDamping(AngularDamping);
		MeshComponent->SetEnableGravity(true);

		// 물리 시뮬레이션 관련 설정 (BeginPlay에서 안전하게 호출)
		if (!HasAnyFlags(RF_ClassDefaultObject))
		{
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetMassOverrideInKg(NAME_None, Weight * 500.0f);
			MeshComponent->SetNotifyRigidBodyCollision(true); // 충돌 이벤트 활성화
			MeshComponent->GetBodyInstance()->bLockXRotation = true;
			MeshComponent->GetBodyInstance()->bLockYRotation = true;
			MeshComponent->GetBodyInstance()->bLockZRotation = true; // Z축 회전도 잠금
			MeshComponent->GetBodyInstance()->bLockTranslation = false;
		}
	}
}

void AHoldingObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 항상 회전을 0,0,0으로 강제 (모든 상황에서 회전 불가)
	SetActorRotation(FRotator::ZeroRotator);
	if (MeshComponent)
	{
		MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	// Z축 위치 체크 (서버에서만) - 초기 위치에서 일정 거리 이상 아래로 떨어지면 리스폰
	if (HasAuthority() && (InitialLocation.Z - GetActorLocation().Z) > RespawnZThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HoldingObject] %s fell below threshold (Initial: %.2f, Current: %.2f, Threshold: %.2f), respawning..."),
			*GetName(), InitialLocation.Z, GetActorLocation().Z, RespawnZThreshold);
		RespawnToInitialLocation();
		return; // 리스폰 후 나머지 로직 건너뛰기
	}

	// 잡힌 상태에서는 직접 위치를 고정 (물고 있는 느낌)
	if (bIsBeingHeld && HoldingCat)
	{
		// 목표 위치 계산 (BiteCat 기준 HoldOffset 적용)
		FVector TargetLocation = HoldingCat->GetActorLocation() +
								HoldingCat->GetActorForwardVector() * HoldOffset.X +
								HoldingCat->GetActorRightVector() * HoldOffset.Y +
								HoldingCat->GetActorUpVector() * HoldOffset.Z;

		// 위치 설정 (회전은 위에서 이미 강제)
		SetActorLocation(TargetLocation);

		// 속도도 초기화하여 관성 제거
		if (MeshComponent)
		{
			MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		}
	}
	else if (!bIsBeingHeld && MeshComponent)
	{
		// 잡히지 않은 상태에서 안정성을 위한 속도 댐핑
		FVector CurrentVelocity = MeshComponent->GetPhysicsLinearVelocity();
		FVector CurrentAngularVelocity = MeshComponent->GetPhysicsAngularVelocityInDegrees();

		// 캐릭터의 밀림만 최소화하고 중력과 던지기는 정상 작동하도록 조정
		// 수평 방향의 작은 움직임만 댐핑 (중력은 수직이므로 영향 없음)
		if (CurrentVelocity.Size() < 50.0f && CurrentVelocity.Size() > 0.1f)
		{
			// 수평 방향만 댐핑, 수직(중력) 방향은 유지
			FVector HorizontalVelocity = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
			FVector VerticalVelocity = FVector(0.0f, 0.0f, CurrentVelocity.Z);

			if (HorizontalVelocity.Size() < 20.0f)
			{
				FVector DampedHorizontal = HorizontalVelocity * FMath::Pow(0.8f, DeltaTime * StabilityDamping);
				MeshComponent->SetPhysicsLinearVelocity(DampedHorizontal + VerticalVelocity);
			}
		}

		// 낮은 수평 속도에서만 정지 (중력 낙하는 유지)
		FVector HorizontalOnly = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
		if (HorizontalOnly.Size() < 2.0f)
		{
			MeshComponent->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, CurrentVelocity.Z));
		}
	}
}

bool AHoldingObject::CanBeHeld() const
{
	return !bIsBeingHeld;
}

void AHoldingObject::OnHeld(ABiteCat* Cat)
{
	if (!CanBeHeld() || !Cat)
	{
		return;
	}

	// 직접 처리 (BiteCat의 서버 RPC에서 호출됨)
	bIsBeingHeld = true;
	HoldingCat = Cat;

	// 잡혀있을 때 캐릭터와 충돌하지 않도록 설정
	if (MeshComponent)
	{
		MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}

	UE_LOG(LogTemp, Warning, TEXT("Object is now being held by BiteCat"));
}

void AHoldingObject::OnReleased()
{
	if (!bIsBeingHeld)
	{
		return;
	}

	// 직접 처리 (BiteCat의 서버 RPC에서 호출됨)
	bIsBeingHeld = false;
	HoldingCat = nullptr;

	// 놓았을 때 캐릭터와 충돌하도록 복구
	if (MeshComponent)
	{
		MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	}

	UE_LOG(LogTemp, Warning, TEXT("Object released from BiteCat"));
}

void AHoldingObject::RespawnToInitialLocation()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	// 잡혀있는 상태라면 먼저 해제
	if (bIsBeingHeld)
	{
		OnReleased();
	}

	// 시작 위치와 기본 회전(0,0,0)으로 리스폰
	SetActorLocationAndRotation(InitialLocation, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);

	// 물리 속도 초기화
	if (MeshComponent)
	{
		MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	UE_LOG(LogTemp, Log, TEXT("HoldingObject respawned to initial location: %s"), *InitialLocation.ToString());
}

