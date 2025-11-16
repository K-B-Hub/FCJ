// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Triggers/PlayerOverlapTrigger.h"
#include "PlayerCharacter/CatBase.h"

APlayerOverlapTrigger::APlayerOverlapTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// 베이스 클래스에서 생성된 TriggerBox의 collision 설정 조정
	// (PlayerOverlapTrigger는 Pawn 채널에만 Overlap 응답)
	if (TriggerBox)
	{
		TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		TriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerOverlapTrigger::OnTriggerBeginOverlap);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &APlayerOverlapTrigger::OnTriggerEndOverlap);
	}
}

void APlayerOverlapTrigger::BeginPlay()
{
	Super::BeginPlay();
}

bool APlayerOverlapTrigger::GetInternalTriggerState_Implementation() const
{
	// 플레이어가 오버랩되어 있으면 true
	return OverlappingPlayers.Num() > 0;
}

void APlayerOverlapTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 오버랩 처리 (리플리케이션을 통해 클라이언트 동기화)
	if (!HasAuthority())
	{
		return;
	}

	ACatBase* Player = Cast<ACatBase>(OtherActor);
	if (Player && !OverlappingPlayers.Contains(Player))
	{
		OverlappingPlayers.Add(Player);
		// 현재 내부 상태를 전달 (배열에 플레이어가 있으므로 true)
		SetTriggerActive(GetInternalTriggerState());
	}
}

void APlayerOverlapTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	// 서버에서만 오버랩 처리 (리플리케이션을 통해 클라이언트 동기화)
	if (!HasAuthority())
	{
		return;
	}

	ACatBase* Player = Cast<ACatBase>(OtherActor);
	if (Player)
	{
		OverlappingPlayers.Remove(Player);
		// 현재 내부 상태를 전달 (다른 플레이어가 남아있을 수 있음)
		SetTriggerActive(GetInternalTriggerState());
	}
}
