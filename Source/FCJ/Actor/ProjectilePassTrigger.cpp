// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/ProjectilePassTrigger.h"
#include "Actor/Projectile.h"
#include "Components/BoxComponent.h"

AProjectilePassTrigger::AProjectilePassTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// 트리거 박스 생성
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AProjectilePassTrigger::OnTriggerBeginOverlap);
}

void AProjectilePassTrigger::BeginPlay()
{
	Super::BeginPlay();
}

bool AProjectilePassTrigger::GetInternalTriggerState_Implementation() const
{
	// 발사체가 지나갔으면 true
	return bProjectilePassed;
}

void AProjectilePassTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AProjectile* Projectile = Cast<AProjectile>(OtherActor);
	if (Projectile)
	{
		// 발사체가 지나가면 영구적으로 true로 설정
		bProjectilePassed = true;
	}
}

void AProjectilePassTrigger::ResetTrigger()
{
	if (bCanReset)
	{
		bProjectilePassed = false;
	}
}
