// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/PlayerVisitedTrigger.h"
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
	ACatBase* Player = Cast<ACatBase>(OtherActor);
	if (Player)
	{
		// 플레이어가 방문하면 영구적으로 true로 설정
		bPlayerVisited = true;
	}
}

void APlayerVisitedTrigger::ResetTrigger()
{
	if (bCanReset)
	{
		bPlayerVisited = false;
	}
}
