// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredActors/TriggeredTurret.h"
#include "Actor/TriggeredActors/Turret.h"
#include "Actor/Triggers/BaseTrigger.h"
#include "Components/ChildActorComponent.h"

ATriggeredTurret::ATriggeredTurret()
{
	// 델리게이트 기반 이벤트 처리를 사용하므로 Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// Turret 자식 액터 컴포넌트 생성
	TurretComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TurretComponent"));
	TurretComponent->SetupAttachment(RootSceneComponent);
	// Turret 클래스는 Blueprint에서 설정 (Child Actor Class)

	// Trigger 자식 액터 컴포넌트 생성
	TriggerComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TriggerComponent"));
	TriggerComponent->SetupAttachment(RootSceneComponent);
	// Trigger 클래스는 Blueprint에서 설정 (Child Actor Class)

	bLastTriggerState = false;
	CachedTurret = nullptr;
	CachedTrigger = nullptr;
}

void ATriggeredTurret::BeginPlay()
{
	Super::BeginPlay();

	// 자식 액터들을 캐시
	if (TurretComponent)
	{
		CachedTurret = Cast<ATurret>(TurretComponent->GetChildActor());
	}

	if (TriggerComponent)
	{
		CachedTrigger = Cast<ABaseTrigger>(TriggerComponent->GetChildActor());
	}

	// 트리거 델리게이트 바인딩
	if (CachedTrigger)
	{
		CachedTrigger->OnTriggerStateChanged.AddDynamic(this, &ATriggeredTurret::OnTriggerStateChangedCallback);

		// 초기 트리거 상태 확인
		if (bAutoCheckTrigger)
		{
			CheckTriggerState();
		}
	}
}

void ATriggeredTurret::CheckTriggerState()
{
	if (!CachedTrigger || !CachedTurret)
		return;

	bool bCurrentTriggerState = CachedTrigger->IsTriggerActive();

	// 트리거 상태가 변경되었을 때만 포탑 상태 업데이트
	if (bCurrentTriggerState != bLastTriggerState)
	{
		bLastTriggerState = bCurrentTriggerState;

		// 트리거 상태에 따라 포탑 활성화/비활성화
		if (bCurrentTriggerState)
		{
			// 트리거가 활성화되면 포탑을 활성화 모드로 전환
			// Turret 자체에서 플레이어 감지 시 발사 시작
			CachedTurret->SetActive(true);
		}
		else
		{
			// 트리거가 비활성화되면 포탑도 즉시 비활성화
			CachedTurret->SetActive(false);
		}
	}
}

void ATriggeredTurret::OnTriggerStateChangedCallback(bool bNewState)
{
	if (!CachedTurret)
		return;

	// 델리게이트를 통해 트리거 상태 변경 이벤트를 받으면 즉시 포탑 상태 업데이트
	bLastTriggerState = bNewState;
	CachedTurret->SetActive(bNewState);
}

