// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/BaseTrigger.h"

ABaseTrigger::ABaseTrigger()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsActive = false;
}

void ABaseTrigger::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ABaseTrigger::IsTriggerActive_Implementation() const
{
	// 내부 트리거 상태를 가져와서 반전 여부에 따라 반환
	bool InternalState = GetInternalTriggerState();
	return bInvertTrigger ? !InternalState : InternalState;
}

bool ABaseTrigger::GetInternalTriggerState_Implementation() const
{
	// 기본 구현은 bIsActive 값을 반환합니다.
	// 파생 클래스에서 오버라이드하여 복잡한 조건을 구현할 수 있습니다.
	return bIsActive;
}

void ABaseTrigger::SetTriggerActive(bool bNewActive)
{
	bIsActive = bNewActive;
}

