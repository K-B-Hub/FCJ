// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Triggers/HoldingObjectOverlapTrigger.h"
#include "Actor/Objects/HoldingObject.h"

AHoldingObjectOverlapTrigger::AHoldingObjectOverlapTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// 베이스 클래스에서 생성된 TriggerBox의 collision 설정 조정
	// (HoldingObjectOverlapTrigger는 모든 채널에 대해 Overlap 응답)
	if (TriggerBox)
	{
		TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AHoldingObjectOverlapTrigger::OnTriggerBeginOverlap);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AHoldingObjectOverlapTrigger::OnTriggerEndOverlap);
	}
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
	// 서버에서만 오버랩 처리 (리플리케이션을 통해 클라이언트 동기화)
	if (!HasAuthority())
	{
		return;
	}

	AHoldingObject* HoldingObj = Cast<AHoldingObject>(OtherActor);
	if (HoldingObj && !OverlappingObjects.Contains(HoldingObj))
	{
		OverlappingObjects.Add(HoldingObj);
		// 현재 내부 상태를 전달 (배열에 객체가 있으므로 true)
		SetTriggerActive(GetInternalTriggerState());
	}
}

void AHoldingObjectOverlapTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	// 서버에서만 오버랩 처리 (리플리케이션을 통해 클라이언트 동기화)
	if (!HasAuthority())
	{
		return;
	}

	AHoldingObject* HoldingObj = Cast<AHoldingObject>(OtherActor);
	if (HoldingObj)
	{
		OverlappingObjects.Remove(HoldingObj);
		// 현재 내부 상태를 전달 (다른 객체가 남아있을 수 있음)
		SetTriggerActive(GetInternalTriggerState());
	}
}
