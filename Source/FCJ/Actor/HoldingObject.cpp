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
	// Pawn과도 충돌하도록 설정 (일반적인 물체처럼)
	
	// Enable physics simulation
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetMassOverrideInKg(NAME_None, Weight * 10.0f); // Weight를 실제 물리 질량으로 사용
	CollisionComponent->SetLinearDamping(LinearDamping); // 높은 선형 댐핑으로 안정성 증가
	CollisionComponent->SetAngularDamping(AngularDamping); // 각속도 댐핑도 추가

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
		CollisionComponent->SetMassOverrideInKg(NAME_None, Weight * 10.0f);
		CollisionComponent->SetLinearDamping(LinearDamping);
		CollisionComponent->SetAngularDamping(AngularDamping);
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

