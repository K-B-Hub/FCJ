// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Triggers/ProjectilePassTrigger.h"
#include "Actor/Objects/Projectile.h"
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
	// 서버에서만 오버랩 처리 (리플리케이션을 통해 클라이언트 동기화)
	if (!HasAuthority())
	{
		return;
	}

	AProjectile* Projectile = Cast<AProjectile>(OtherActor);
	if (Projectile && !bProjectilePassed)
	{
		// 발사체가 지나가면 영구적으로 true로 설정
		bProjectilePassed = true;
		SetTriggerActive(GetInternalTriggerState());
	}
}

void AProjectilePassTrigger::ResetTrigger()
{
	// 서버에서만 리셋 가능
	if (!HasAuthority())
	{
		return;
	}

	if (bCanReset && bProjectilePassed)
	{
		bProjectilePassed = false;
		// 상태 변경 이벤트 발생
		SetTriggerActive(GetInternalTriggerState());
	}
}
