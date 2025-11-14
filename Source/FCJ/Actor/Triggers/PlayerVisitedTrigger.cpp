// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Triggers/PlayerVisitedTrigger.h"
#include "PlayerCharacter/CatBase.h"
#include "Components/BoxComponent.h"

APlayerVisitedTrigger::APlayerVisitedTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// 트리거 박스 생성
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerVisitedTrigger::OnTriggerBeginOverlap);
}

void APlayerVisitedTrigger::BeginPlay()
{
	Super::BeginPlay();
}

bool APlayerVisitedTrigger::GetInternalTriggerState_Implementation() const
{
	// 플레이어가 방문했으면 true
	return bPlayerVisited;
}

void APlayerVisitedTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 오버랩 처리 (리플리케이션을 통해 클라이언트 동기화)
	if (!HasAuthority())
	{
		return;
	}

	ACatBase* Player = Cast<ACatBase>(OtherActor);
	if (Player && !bPlayerVisited)
	{
		// 플레이어가 방문하면 영구적으로 true로 설정
		bPlayerVisited = true;
		SetTriggerActive(GetInternalTriggerState());
	}
}

void APlayerVisitedTrigger::ResetTrigger()
{
	// 서버에서만 리셋 가능
	if (!HasAuthority())
	{
		return;
	}

	if (bCanReset && bPlayerVisited)
	{
		bPlayerVisited = false;
		// 상태 변경 이벤트 발생
		SetTriggerActive(GetInternalTriggerState());
	}
}
