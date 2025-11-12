// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/HoldingObjectOverlapTrigger.h"
#include "Actor/HoldingObject.h"
#include "Components/BoxComponent.h"

AHoldingObjectOverlapTrigger::AHoldingObjectOverlapTrigger()
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

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AHoldingObjectOverlapTrigger::OnTriggerBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AHoldingObjectOverlapTrigger::OnTriggerEndOverlap);
}

void AHoldingObjectOverlapTrigger::BeginPlay()
{
	Super::BeginPlay();
}

bool AHoldingObjectOverlapTrigger::GetInternalTriggerState_Implementation() const
{
	// HoldingObject가 오버랩되어 있으면 true
	return OverlappingObjects.Num() > 0;
}

void AHoldingObjectOverlapTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHoldingObject* HoldingObj = Cast<AHoldingObject>(OtherActor);
	if (HoldingObj && !OverlappingObjects.Contains(HoldingObj))
	{
		OverlappingObjects.Add(HoldingObj);
	}
}

void AHoldingObjectOverlapTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	AHoldingObject* HoldingObj = Cast<AHoldingObject>(OtherActor);
	if (HoldingObj)
	{
		OverlappingObjects.Remove(HoldingObj);
	}
}
