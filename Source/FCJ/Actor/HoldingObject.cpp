// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/HoldingObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter/BiteCat.h"
#include "Engine/Engine.h"

AHoldingObject::AHoldingObject()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetBoxExtent(BoxExtent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	// Pawn과는 충돌하지만 물리적 밀림은 최소화
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	
	// Enable physics simulation
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetMassOverrideInKg(NAME_None, Weight * 500.0f); // 극도로 무거운 질량으로 설정하여 밀림 방지
	CollisionComponent->SetLinearDamping(LinearDamping); // 높은 선형 댐핑으로 안정성 증가
	CollisionComponent->SetAngularDamping(AngularDamping); // 각속도 댐핑도 추가
	
	// 추가 물리 안정성 설정
	CollisionComponent->SetEnableGravity(true);
	CollisionComponent->GetBodyInstance()->bLockXRotation = true; // X축 회전 잠금
	CollisionComponent->GetBodyInstance()->bLockYRotation = true; // Y축 회전 잠금
	// Z축은 필요에 따라 자유롭게 회전 가능하도록 유지
	
	// 캐릭터의 밀림을 극도로 제한하기 위한 설정
	CollisionComponent->GetBodyInstance()->bLockTranslation = false; // 완전 잠금은 아니지만
	CollisionComponent->SetNotifyRigidBodyCollision(true); // 충돌 이벤트 활성화

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Initialize holding state
	bIsBeingHeld = false;
	HoldingCat = nullptr;
}

void AHoldingObject::BeginPlay()
{
	Super::BeginPlay();
	
	// Apply blueprint settings to collision component
	if (CollisionComponent)
	{
		CollisionComponent->SetBoxExtent(BoxExtent);
		CollisionComponent->SetMassOverrideInKg(NAME_None, Weight * 500.0f); // 극도로 무거운 질량 유지
		CollisionComponent->SetLinearDamping(LinearDamping);
		CollisionComponent->SetAngularDamping(AngularDamping);
		
		// 추가 안정성 설정 적용
		CollisionComponent->GetBodyInstance()->bLockXRotation = true;
		CollisionComponent->GetBodyInstance()->bLockYRotation = true;
		CollisionComponent->SetEnableGravity(true);
	}
}

void AHoldingObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 잡힌 상태에서는 직접 위치를 고정 (물고 있는 느낌)
	if (bIsBeingHeld && HoldingCat)
	{
		// 목표 위치 계산 (BiteCat 기준 HoldOffset 적용)
		FVector TargetLocation = HoldingCat->GetActorLocation() + 
								HoldingCat->GetActorForwardVector() * HoldOffset.X + 
								HoldingCat->GetActorRightVector() * HoldOffset.Y + 
								HoldingCat->GetActorUpVector() * HoldOffset.Z;
		
		// 직접 위치 설정 - 물고 있는 것처럼 완전 고정
		SetActorLocation(TargetLocation);
		
		// 속도도 초기화하여 관성 제거
		if (CollisionComponent)
		{
			CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
			CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}
	}
	else if (!bIsBeingHeld && CollisionComponent)
	{
		// 잡히지 않은 상태에서 안정성을 위한 속도 댐핑
		FVector CurrentVelocity = CollisionComponent->GetPhysicsLinearVelocity();
		FVector CurrentAngularVelocity = CollisionComponent->GetPhysicsAngularVelocityInDegrees();
		
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
				CollisionComponent->SetPhysicsLinearVelocity(DampedHorizontal + VerticalVelocity);
			}
		}
		
		// 낮은 수평 속도에서만 정지 (중력 낙하는 유지)
		FVector HorizontalOnly = FVector(CurrentVelocity.X, CurrentVelocity.Y, 0.0f);
		if (HorizontalOnly.Size() < 2.0f)
		{
			CollisionComponent->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, CurrentVelocity.Z));
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

	bIsBeingHeld = true;
	HoldingCat = Cat;

	// 잡힌 상태에서는 물리 시뮬레이션 유지 (놓을 때를 위해)
	// 위치는 Tick에서 직접 설정됨

	UE_LOG(LogTemp, Warning, TEXT("Object is now being held by BiteCat"));
}

void AHoldingObject::OnReleased()
{
	if (!bIsBeingHeld)
	{
		return;
	}

	bIsBeingHeld = false;
	HoldingCat = nullptr;

	// 놓을 때는 물리 시뮬레이션이 자연스럽게 이어짐
	// 댐핑값은 BeginPlay에서 설정된 원래값 유지

	UE_LOG(LogTemp, Warning, TEXT("Object released from BiteCat"));
}

