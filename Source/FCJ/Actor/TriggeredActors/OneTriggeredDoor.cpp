// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredActors/OneTriggeredDoor.h"
#include "Actor/TriggeredActors/MovingDoor.h"
#include "Actor/Triggers/BaseTrigger.h"
#include "Components/ChildActorComponent.h"

AOneTriggeredDoor::AOneTriggeredDoor()
{
	// 델리게이트 기반 이벤트 처리를 사용하므로 Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// Door 자식 액터 컴포넌트 생성
	DoorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorComponent"));
	DoorComponent->SetupAttachment(RootSceneComponent);
	// Door 클래스는 Blueprint에서 설정 (Child Actor Class)

	// Trigger 자식 액터 컴포넌트 생성
	TriggerComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TriggerComponent"));
	TriggerComponent->SetupAttachment(RootSceneComponent);
	// Trigger 클래스는 Blueprint에서 설정 (Child Actor Class)

	bDoorOpened = false;
	CachedDoor = nullptr;
	CachedTrigger = nullptr;
}

void AOneTriggeredDoor::BeginPlay()
{
	Super::BeginPlay();

	// 자식 액터들을 캐시
	if (DoorComponent)
	{
		CachedDoor = Cast<AMovingDoor>(DoorComponent->GetChildActor());
	}

	if (TriggerComponent)
	{
		CachedTrigger = Cast<ABaseTrigger>(TriggerComponent->GetChildActor());
	}

	// 트리거 델리게이트 바인딩
	if (CachedTrigger)
	{
		CachedTrigger->OnTriggerStateChanged.AddDynamic(this, &AOneTriggeredDoor::OnTriggerStateChanged);
	}
}

void AOneTriggeredDoor::OnTriggerStateChanged(bool bNewState)
{
	// 이미 문이 열렸으면 더 이상 체크하지 않음
	if (bDoorOpened)
	{
		return;
	}

	// 트리거가 활성화되면 문 열기
	if (bNewState)
	{
		if (CachedDoor)
		{
			// 문 열기 (서버에서만 실행됨)
			CachedDoor->OpenDoor();
			bDoorOpened = true;

			// 델리게이트 바인딩 해제 (더 이상 필요 없음)
			if (CachedTrigger)
			{
				CachedTrigger->OnTriggerStateChanged.RemoveDynamic(this, &AOneTriggeredDoor::OnTriggerStateChanged);
			}
		}
	}
}
