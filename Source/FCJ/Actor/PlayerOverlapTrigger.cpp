// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/PlayerOverlapTrigger.h"
#include "PlayerCharacter/CatBase.h"
#include "Components/BoxComponent.h"

APlayerOverlapTrigger::APlayerOverlapTrigger()
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

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerOverlapTrigger::OnTriggerBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &APlayerOverlapTrigger::OnTriggerEndOverlap);
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
	ACatBase* Player = Cast<ACatBase>(OtherActor);
	if (Player && !OverlappingPlayers.Contains(Player))
	{
		OverlappingPlayers.Add(Player);
	}
}

void APlayerOverlapTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	ACatBase* Player = Cast<ACatBase>(OtherActor);
	if (Player)
	{
		OverlappingPlayers.Remove(Player);
	}
}
