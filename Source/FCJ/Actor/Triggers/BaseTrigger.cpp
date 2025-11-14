// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Triggers/BaseTrigger.h"
#include "Net/UnrealNetwork.h"

ABaseTrigger::ABaseTrigger()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsActive = false;

	// 네트워크 리플리케이션 활성화
	bReplicates = true;
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
	// 서버에서만 상태 변경 가능
	if (!HasAuthority())
	{
		return;
	}

	// 상태가 실제로 변경될 때만 델리게이트 브로드캐스트
	if (bIsActive != bNewActive)
	{
		bIsActive = bNewActive;

		// 반전 적용된 최종 상태 계산
		bool bFinalState = bInvertTrigger ? !bIsActive : bIsActive;

		// 서버에서 트리거 상태 변경 이벤트 브로드캐스트
		OnTriggerStateChanged.Broadcast(bFinalState);
	}
}

void ABaseTrigger::OnRep_IsActive()
{
	// 클라이언트에서 리플리케이션을 통해 상태가 변경되었을 때 델리게이트 브로드캐스트
	bool bFinalState = bInvertTrigger ? !bIsActive : bIsActive;
	OnTriggerStateChanged.Broadcast(bFinalState);
}

void ABaseTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// bIsActive를 모든 클라이언트에 리플리케이트
	DOREPLIFETIME(ABaseTrigger, bIsActive);
}

