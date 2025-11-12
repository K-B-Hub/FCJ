// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredTurret.h"
#include "Actor/Turret.h"
#include "Actor/BaseTrigger.h"
#include "Components/ChildActorComponent.h"

ATriggeredTurret::ATriggeredTurret()
{
	PrimaryActorTick.bCanEverTick = true;

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
	TimeSinceLastCheck = 0.0f;
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

	// 초기 트리거 상태 확인
	if (CachedTrigger && bAutoCheckTrigger)
	{
		CheckTriggerState();
	}
}

void ATriggeredTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 자동 트리거 체크가 활성화되어 있고 트리거가 설정되어 있으면
	if (bAutoCheckTrigger && CachedTrigger && CachedTurret)
	{
		TimeSinceLastCheck += DeltaTime;

		if (TimeSinceLastCheck >= TriggerCheckInterval)
		{
			CheckTriggerState();
			TimeSinceLastCheck = 0.0f;
		}

		// 트리거가 비활성화 상태인데 Turret이 활성화되어 있으면 강제로 비활성화
		// (Turret의 자동 활성화를 방지)
		bool bCurrentTriggerState = CachedTrigger->IsTriggerActive();
		if (!bCurrentTriggerState && CachedTurret->IsActive())
		{
			CachedTurret->SetActive(false);
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

